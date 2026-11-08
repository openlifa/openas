#!/usr/bin/env python3
"""Check the actual source/core-API/static-archive public module boundary."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def consumer_source(conformance: bool) -> str:
    imports = 'import std;\nimport openas;\n'
    if conformance:
        imports += 'import openas.conformance;\n'
    checks = '''
    auto actor = openas::parse("agent:reviewer;via=person:owner");
    if (!actor || openas::parse(openas::format(*actor)) != actor) return 1;
    auto grant = openas::parse_grant("agent:reviewer;via=person:owner=reversible@document.*");
    if (!grant || !grant->covers("document.save") || grant->covers("other.save")) return 2;
    openas::Verb save{.name="document.save", .summary="Save the document", .effect=openas::Effect::reversible};
    if (openas::decide(save, *actor, false, {}, std::span{&*grant, 1}) != openas::Decision::run) return 3;
    if (openas::decide(save, *actor, false, openas::Cap::none(), std::span{&*grant, 1}) != openas::Decision::refuse) return 4;
'''
    if conformance:
        checks += '''
    if (!openas::conformance::check(*actor).ok() || !openas::conformance::check(*grant).ok()) return 5;
    if (!openas::conformance::check(std::span{&save, 1}).ok()) return 6;
    auto invalid = *grant; invalid.scope = {"document.save_now"};
    if (openas::conformance::check(invalid).ok()) return 7;
    save.effect = openas::Effect::irreversible;
    if (openas::conformance::check(std::span{&save, 1}).ok()) return 8;
'''
    return imports + 'int main() {\n' + checks + '\n    std::println("openas distribution: ok");\n}\n'


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--mcpp', default=os.environ.get('MCPP', 'mcpp'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--keep-work', action='store_true')
    args = parser.parse_args()
    source = args.source_root.resolve()
    output = args.output.resolve(); output.parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='openas-distribution-'))
    artifacts = output.parent / (output.stem + '-artifacts'); artifacts.mkdir(parents=True, exist_ok=True)
    report = {'format':'openas.distribution@0.1', 'ok':False, 'source':str(source), 'work':str(work),
              'commands':[], 'consumers':[], 'source_sha256':{str(p.relative_to(source)):digest(p)
                  for p in [source/'mcpp.toml', *sorted((source/'src').glob('*'))] if p.is_file()}}

    def run(project: Path, label: str, argv: list[str], bootstrap: bool = False) -> str:
        env = os.environ.copy(); env.setdefault('MCPP_JOBS', '2')
        if bootstrap: env.pop('MCPP_LOCKED', None)
        else: env['MCPP_LOCKED'] = '1'
        result = subprocess.run([args.mcpp, *argv], cwd=project, env=env, capture_output=True, text=True)
        log = artifacts/(label+'.log'); log.write_text(result.stdout+'\nSTDERR\n'+result.stderr)
        report['commands'].append({'label':label, 'argv':[args.mcpp, *argv], 'exit':result.returncode, 'log':str(log)})
        output.write_text(json.dumps(report, indent=2)+'\n')
        if result.returncode: raise RuntimeError(label+' failed: '+str(log))
        if "not one of that package's public modules" in result.stderr:
            raise RuntimeError(label+' imported a private dependency module')
        return result.stdout

    def consume(name: str, product: Path, conformance: bool, sdk: bool = False) -> None:
        project = work/name; (project/'src').mkdir(parents=True)
        selection = ''
        (project/'mcpp.toml').write_text(
            '[package]\nnamespace="openas.validation"\nname="consumer"\nversion="0.1.0"\n'
            '[targets.consumer]\nkind="bin"\nmain="src/main.cpp"\n'
            '[build]\nsources=["src/*.cpp"]\n[dependencies]\n'
            'openlifa.openas = { path = '+json.dumps(str(product))+selection+' }\n')
        code = consumer_source(conformance)
        if sdk: code = code.replace('import openas;\nimport openas.conformance;\n', 'import openas.sdk;\n')
        (project/'src/main.cpp').write_text(code)
        run(project, name+'-build', ['build'], True)
        result = run(project, name+'-run', ['run', '--locked'])
        if 'openas distribution: ok' not in result: raise RuntimeError(name+' did not execute the checks')
        resolution = max((project/'target').glob('*/*/resolution.json'), key=lambda p:p.stat().st_mtime_ns)
        cdb = resolution.parent/'compile_commands.json'; commands = json.loads(cdb.read_text()); files=[]
        for command in commands:
            directory=Path(command['directory']); directory=directory if directory.is_absolute() else project/directory
            path=Path(command['file']); path=(path if path.is_absolute() else directory/path).resolve(); files.append(path)
        if name!='source' and any(p.is_relative_to(work/'producer') or p.is_relative_to(source) for p in files):
            raise RuntimeError('packed consumer compiled original source')
        shutil.copyfile(resolution,artifacts/(name+'.resolution.json')); shutil.copyfile(cdb,artifacts/(name+'.compile_commands.json'))
        report['consumers'].append({'name':name,'conformance':conformance,'sdk_import':sdk,'compiled_units':len(files),'ok':True})

    try:
        producer = work/'producer'; producer.mkdir()
        shutil.copyfile(source/'mcpp.toml', producer/'mcpp.toml'); shutil.copytree(source/'src', producer/'src')
        for name in ('README.md','LICENSE','LICENSE-SPEC','SPEC.md'):
            if (source/name).is_file(): shutil.copyfile(source/name, producer/name)
        run(producer,'producer-build',['build','--profile','release'],True)
        packed = run(producer,'pack',['pack','--profile','release','--locked','--message-format','json'])
        envelopes=[]
        for offset,char in enumerate(packed):
            if char!='{': continue
            try: value,_=json.JSONDecoder().raw_decode(packed[offset:])
            except ValueError: continue
            if isinstance(value,dict) and value.get('kind')=='mcpp.pack': envelopes.append(value)
        if len(envelopes)!=1: raise RuntimeError('missing unique pack envelope')
        archive=Path(envelopes[0]['data']['artifacts'][0]['path'])
        installation=work/'installation'; installation.mkdir()
        with tarfile.open(archive) as package: package.extractall(installation,filter='data')
        manifests=list(installation.rglob('mcpp.toml'))
        if len(manifests)!=1: raise RuntimeError('unexpected packed manifest set')
        installed=manifests[0].parent
        if list(installed.rglob('*.cpp')): raise RuntimeError('archive contains implementation sources')
        saved=artifacts/archive.name; shutil.copyfile(archive,saved)
        report['packed']={'archive':str(saved),'sha256':digest(saved),'bytes':saved.stat().st_size,
                          'interfaces':sorted(str(p.relative_to(installed)) for p in installed.rglob('*.cppm'))}
        # Test the actual archive first: an old source-only conformance module
        # must fail here, not be rescued by a path back to the producer.
        consume('packed',installed,True)
        consume('source',producer,True)
        consume('core-api',installed,False)
        consume('sdk-api',installed,True,True)
        for path,expected in report['source_sha256'].items():
            if digest(source/path)!=expected: raise RuntimeError('source changed during validation: '+path)
        report['ok']=True
    except (OSError,RuntimeError,ValueError,KeyError,tarfile.TarError) as error:
        report['error']=str(error)
    finally:
        report['work_retained']=args.keep_work
        output.write_text(json.dumps(report,indent=2)+'\n')
        if not args.keep_work: shutil.rmtree(work)
    print(json.dumps({'ok':report['ok'],'report':str(output),'error':report.get('error')}))
    return 0 if report['ok'] else 1


if __name__=='__main__': raise SystemExit(main())
