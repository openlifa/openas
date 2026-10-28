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

## License

Code: [Apache-2.0](LICENSE). The specification text: [CC BY-SA 4.0](LICENSE-SPEC).
Copyright 2026 OpenLifa & Sunrisepeak.
