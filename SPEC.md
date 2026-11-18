# The openas specification, version 0.2

Status: draft. This document is normative. The key words MUST, MUST NOT,
SHOULD and MAY are used as in RFC 2119.

Version 0.2 settles the actor's text form (§2.4), adds presence (§2.7), the
person's own verbs (§3.4), grants and a system's cap (§5), exit code 3 and the
error codes (§6), and the conformance of actors and grants (§7). What 0.1
defined keeps its meaning; nothing was removed.

## 1. Scope

openas describes actors — people, agents, programs and services — and what
they do with a system: who did it and for whom, what kind of effect it had,
who may do it without asking, and how a run is reported. A system that serves
several kinds of actor describes its verbs once and reports every run the
same way; a person and an agent differ in who answers a question and in what
they were granted, not in which verbs they have.

## 2. Actors

2.1 An actor has a kind — `person`, `agent`, `program`, `service` — and MAY
have a name.

2.2 An actor MAY act for others. `via` lists them, the outermost first: an
agent a person started is `agent:planner;via=person:alice`; a helper that
agent started is `agent:reader;via=person:alice,agent:planner`.

2.3 Delegation only adds to the chain. An actor acts for every actor in its
chain, and for no one else.

2.4 The text form is `<kind>[:<name>][;<key>=<value>]…`. A name is letters,
digits and `-_.@`. The key `via` appears at most once; its value is one or
more actors in short form (`<kind>[:<name>]`), separated by commas. Other keys
are reserved: a reader MUST skip a key it does not know, and a writer MUST NOT
write one this version does not define.

2.5 An actor is inherited, not declared: what a process starts acts as that
process's actor, or as a delegate of it.

2.6 A record names how it knows its actor: *given* (by whoever started the
run, on its command line or in its request), *inherited* (from the process
that started it) or *observed* (what the system can tell of itself). An actor
is as trustworthy as the boundary between it and the record: a record its
process cannot write (another environment, another user) holds against it;
one it can write does not.

2.7 A person is *present* when the system observes the person's own input —
at the machine, through the system's own means of input. A person that is
given or inherited (§2.6) is not present, whatever name it carries. Only the
system decides presence; a request MUST NOT be able to claim it.

## 3. Effects

3.1 Every verb has one effect class, in increasing order:

| Effect | Meaning |
|---|---|
| `read` | changes nothing |
| `reversible` | changes what the system can take back |
| `irreversible` | removes or overwrites what cannot be taken back |
| `external` | reaches outside the system: the host, the network, a device |

A reader that meets an effect it does not know MUST treat it as `external`.

3.2 A verb whose effect is `irreversible` or `external` asks before it runs,
unless the one running it has already answered or holds a grant for it (§5).

3.3 A run that needs an answer no one present can give ends with exit code 2
and names the option that answers it. It never waits.

3.4 A verb MAY be the person's own (`person`): the answer to a question an
agent asked, a permission, a payment's approval. It runs only for a present
person (§2.7). A request for it from any other actor MUST be refused (error
code `refused`, exit code 1) and change nothing. No grant and no delegation
reaches it.

## 4. Verbs

A system describes each verb it offers: its name, a summary, its effect,
whether it is the person's own, its arguments (by position), its options (by
name; a flag takes no value) and the options that answer its questions.

## 5. Grants

5.1 A system MAY have a *cap*: the highest effect a verb may have and still be
run for an actor other than a present person. A cap of `none` lets no such
actor run any verb. A system's cap is set by whoever builds the system, and
nothing at run time raises it.

5.2 A *grant* names an actor, an effect and a scope: a list of verb names,
where a name ending in `.*` covers the verbs beginning with what precedes the
`*`, and `*` covers every verb. A grant holds for the actor it names and for
every actor that acts for it (§2.3).

5.3 For an actor other than a present person, a verb:

1. above the cap is refused (error code `refused`, exit code 1);
2. the person's own (§3.4) is refused;
3. within a grant the actor holds — in its scope, and of an effect no higher
   than the grant's — runs without asking;
4. otherwise runs or asks as §3.2 says.

5.4 A grant is given by a present person and by no one else; an actor never
grants itself or what acts for it. A grant reaches no higher than the cap.

5.5 The text form of a grant is `<actor>=<effect>[@<scope>,<scope>…]`; no
scope is `*`. For example `agent:claude=irreversible@notes.*,files.open`.

## 6. The envelope

6.1 A run is reported as lines, each one JSON object, with `kind`:

| kind | Fields |
|---|---|
| `phase` | `phase`, `state` (`begin`, `done`, `failed`), `text` |
| `progress` | `phase`, `percent` (0..1, or absent), `text` |
| `log` | `level`, `text` |
| `notice` | `text`, `hint` |
| `error` | `code`, `text`, `hint` (the step that helps) |
| `data` | `data_kind`, `data` |
| `result` | `exitCode`; and `actor`, `trace` |

6.2 The last line is `result`. Exit code 0 is done, 1 failed, 2 needs an
answer (§3.3), 3 not ready: the system could not run it yet (what it waits
for is not there), and the same request MAY succeed later.

6.3 An `error`'s `code` is one of:

| Code | Meaning |
|---|---|
| `needs-answer` | §3.3; the record names the options that answer |
| `refused` | not allowed: §3.4, §5.3 |
| `not-found` | what it names does not exist |
| `invalid-argument` | an argument is missing, unknown or of another type |
| `stale` | it names what has changed since it was read |
| `timeout` | what it waited for did not come in time |
| `failed` | it ran and did not succeed |
| `internal` | the system failed |

A reader that meets a code it does not know MUST treat it as `failed`.

6.4 Fields are only ever added. A reader skips a kind or a field it does not
know.

6.5 A description of the verbs is one object: `openas` (this version),
`client`, `version`, `verbs`.

## 7. Conformance

`openas::conformance::check(verbs)` holds that:

7.1 a verb's name is lower case words;

7.2 names are unique;

7.3 every verb says what it does;

7.4 an argument or option is named once;

7.5 an answering option is one of the verb's options;

7.6 a verb that asks first names the option that answers.

`openas::conformance::check(actor)` holds that its text form reads back as
the same actor (7.7), and `check(grant)` that a grant's text form reads back
as the same grant and its scopes are names or `.*` prefixes (7.8).

## 8. Matters this version does not settle

How a grant is stored and carried (a token), its expiry, budgets, stopping an
actor, and the record of runs beyond the `actor` and `trace` of a result.
