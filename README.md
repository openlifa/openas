# openas

Actors and what they do with a system: who did it and for whom, what effect it
has, who may do it without asking, and how a run is reported.

[![CI](https://github.com/openlifa/openas/actions/workflows/ci.yml/badge.svg?event=pull_request)](https://github.com/openlifa/openas/actions/workflows/ci.yml)
[![spec](https://img.shields.io/badge/spec-0.2%20draft-blue)](SPEC.md)
[![mcpp-index](https://img.shields.io/badge/mcpp--index-openlifa.openas%200.2.0-orange)](https://github.com/mcpplibs/mcpp-index)
[![code](https://img.shields.io/badge/code-Apache--2.0-green)](LICENSE)
[![text](https://img.shields.io/badge/text-CC%20BY--SA%204.0-lightgrey)](LICENSE-SPEC)

| | |
|---|---|
| [SPEC.md](SPEC.md) | the specification (normative) |
| [src/](src/) | its C++ modules: `openas`, `openas.conformance` |
| [tests/](tests/) | the modules' tests |

```toml
[dependencies]
openlifa.openas = "0.2.0"
```

```cpp
import openas;

auto alice = *openas::parse("person:alice");
auto agent = openas::delegate(alice, openas::Kind::agent, "claude");
openas::format(agent);            // "agent:claude;via=person:alice"

openas::Verb drop{.name = "notes.delete", .summary = "Delete a note", .effect = openas::Effect::irreversible};
openas::Grant grant = *openas::parse_grant("agent:claude;via=person:alice=irreversible@notes.*");
openas::decide(drop, agent, false, {}, {&grant, 1});   // Decision::run; without the grant, ask
```

Used by [openmui](https://github.com/openlifa/openmui).

## Public C++ entry points

- `import openas;` keeps the actor, effect, grant and envelope API.
- `import openas.conformance;` checks a system description, actor or grant
  against the same specification implementation.
- `import openas.sdk;` re-exports both for clients that want the complete SDK.

The SDK module is the explicit `[lib]` interface root. It is only an acyclic
aggregation: the existing modules and their type identities do not change,
and no checker or protocol rules are duplicated. In particular, the core
module does not import its own conformance client. No additional package or
build-program stage is needed.

Source and archive consumers must see these same public entry points. The
existing indexed 0.2.0 artifact predates this corrected publishing boundary;
use this checkout or a newly built archive to validate it until a release
including the fix is published. This change does not publish or replace the
existing indexed artifact. Standard version 0.2 and runtime behavior are
unchanged. A core-API consumer need only import `openas`; that is not a claim
that conformance source files are pruned from this package's build.

Run the distribution regression without a GUI (Python 3.12+ and mcpp):

```sh
python3 tools/check-distribution.py --output target/distribution.report.openas.json
```

The checker works in temporary producer/consumer projects, leaving the source
checkout and its lock untouched. It builds a real static archive, unpacks it,
then builds and executes separate source, archive, core-API and SDK-API
consumers. Both positive authorization/conformance cases and invalid inputs
are checked. The archive must contain interfaces and compiled implementation,
not `.cpp` sources; packed consumers must not compile the producer's original
sources. A private-module warning is a failure. The JSON report and logs
identify each command and its result; nonzero exit means incomplete validation.
Use `--keep-work` for compiler/graph inspection, and rerun after correcting a
failed declaration. This is a library distribution check, not an installation,
release or remote permission change.

## License

Code: [Apache-2.0](LICENSE). The specification text: [CC BY-SA 4.0](LICENSE-SPEC).
Copyright 2026 OpenLifa & Sunrisepeak.
