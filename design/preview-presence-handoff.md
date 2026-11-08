# Transactions, previews, asking the person, presence and handoff — a proposal for openas 0.3

Status: design draft, not normative. It proposes the interaction model a
system needs when most of what people do reaches it through agents — a
person states an intent, an agent acts, the person looks, points, adjusts
and decides — and records what a first implementation shows. Text that
survives validation moves into SPEC.md; until then nothing here is part of
the standard.

Why openas: these are properties of an actor using a system, not of a user
interface. A game's editor, a simulation's workbench and an application's
window need the same answers to "can it be taken back", "what would it do",
"may I", "who is acting" and "who has it now". Protocols that carry them
(MCP, WebMCP, A2UI, AG-UI, platform intents) are transports and entry points;
this is the model underneath them.

Validation so far: the omui reference implementation (github.com/Sunrisepeak/mui,
branch openmui-design): §1 and §2 as its protocol's `transaction` and
`preview` operations and MCP's `ui_transaction` / `ui_preview`
(`tools/mcp/tests/mcp_test.cpp`); §4 partly, as its agent-presence component;
§3 and §5 are proposed.

## 1. Transactions

1.1 A *transaction* is a list of steps performed as one: all of them done, or
all taken back, last first. Each step is an action an actor could perform on
its own.

1.2 Only steps that can be taken back may be in one: an action whose effect is
`read`, or `reversible` with a known way back (its inverse). A step that asks
the person first (`irreversible`, `external`) or whose way back is unknown is
refused before anything runs — the eligibility is the dry run's (§2.5), so the
answer to "may this be in a transaction" is the same asked or done.

1.3 A transaction that fails part-way takes back what it did. A step that gave
no way back stays done; the answer names it (`kept`). A system MUST NOT say a
transaction was taken back while some of it stays.

1.4 A committed transaction is one step of the system's history: one undo
takes all of it back.

## 2. Previews: shown before it is allowed

2.1 A *preview* answers what a list of steps would make, without leaving
anything of them. The system runs the steps that can be taken back inside a
transaction no one else sees, observes the state they make, and takes them
back.

2.2 Nothing of a preview stays: no record in the system's journal or audit of
actions (the preview itself MAY be audited as a `read`), no step of history, no
change visible to other actors while it runs. A system that cannot preview
without being seen (it presents frames as it goes) MUST say so rather than
preview.

2.3 A step that asks the person first is never run in a preview: it is listed
as not done, with its effect, and what the preview shows is without it.

2.4 A step whose way back is unknown cannot be shown without being done: the
preview is refused and nothing runs.

2.5 A *dry run* (openas 0.2) answers, per step and without running it, what it
would do as far as the system can say statically. A preview runs the steps and
observes the result; it MAY carry what a dry run cannot — the state after, as
the person would see it.

2.6 The answer carries: the steps run; the steps not done (with effects); the
changes as a reader reads them (things added, removed, and changed in name,
value, state, role or description); when the system can render, a picture of
before and of after; and `restored` — whether, after it, the system is as it
was. `restored: false` is an error a system MUST report.

2.7 Presentation is the system's: motion, if any, is held so the after state
shows its end.

## 3. Asking the person

3.1 An actor MAY ask the person for a decision: a *review request* names the
steps (or an intent), carries the preview (§2), and offers options — at least
allow once and decline; a system MAY offer allow for the session (within the
grant), and change (the person adjusts, the actor sees what they changed).

3.2 A review request is the only way an actor obtains a decision it lacks. An
actor MUST NOT perform an action that needs an answer without one, and MUST NOT
answer its own request (an action marked for the person alone is refused to
every request on the actor's channel, whatever actor it names).

3.3 When to ask: an action that needs an answer (irreversible, external);
something the actor is not confident of; a judgement of taste or of the
person's intent. A system SHOULD count requests that should have been made and
were not (an action later undone by the person, a decision taken without an
answer the grant did not cover): the count is a measure of the actor, next to
how often it asked.

3.4 The answer is recorded with the request: who answered, which option, and
the delegation chain (openas 0.2 §5), so the audit says the person allowed it,
not the actor.

## 4. Presence and intent

4.1 An actor using a system has a *presence*: what it is acting on (an object,
a region), what it intends (the next steps, or a note), and its state —
acting, waiting for the person, idle.

4.2 A system SHOULD make an actor's presence observable to the people using it,
in the system's own terms (a highlight round the object, a cursor, a figure);
the presence of an actor the person cannot see is a reason for the person to
stop it (§5).

4.3 Presence is read within the grant: an actor sees the presence of other
actors only where its grant lets it read what they act on.

4.4 Intent is said before acting where the system asks for it (a session's
`intend`, as the real-time draft records): the record of play keeps it beside
the action, and a replay draws it.

## 5. Takeover and handoff

5.1 The person can take over at any time: an actor's session stops at once,
its pending actions cancelled (none taken effect after the stop), and what it
did stays undoable step by step.

5.2 Two actors on one object: a person's direct input wins. An actor's action
on an object the person is operating at that moment (holding it, typing into
it, dragging it) is refused with `conflict`, carrying when to retry; it does
not queue behind the person's.

5.3 Handoff: an actor MAY yield an object or a task to the person (or another
actor) with a summary — what it did, what is left, what it was unsure of — and
the system shows the summary with the presence (§4) it ends.

5.4 Delegation (openas 0.2 §5) is unchanged: handing a task to another actor
never widens what may be done.

## 6. Where the protocols that carry agents stand

What each says about the model above, read from its current text (2026-11-09).
"—" is silence: an implementation may add it, the protocol does not carry it,
so two implementations cannot rely on it together.

| | transaction | preview | undo | asking the person | presence | a person's input wins | takeover | actor and delegation | effect classes |
|---|---|---|---|---|---|---|---|---|---|
| MCP 2026-07-28 | — (no protocol session; state by handles) | — | — | elicitation, `input_required` | — | — | cancellation of a request | OAuth scopes; no actor or chain in a call | hints, untrusted: read-only, destructive, idempotent, open-world |
| MCP Apps (extension) | — | — | — | the app's own UI | — | — | — | as MCP | as MCP |
| WebMCP (CG draft 2026-11-02) | — | — | — | — (no prompt API; the page's own) | — | — | unregistering a tool (`signal`) | — | `readOnlyHint`, `consequentialHint`, `untrustedContentHint` |
| AG-UI | — | — | — | run interrupts, resumed with input | steps and activity events | — | — | sub-agent runs attributed | — |
| A2UI 1.0 (candidate) | — | — | — | it is the UI the agent asks with | — | — | — | — (transport's) | — |
| App Intents (2026) | — | confirmation snippets (the developer's view) | `UndoableIntent` | `requestConfirmation`, multiple choice | — | — | `CancellableIntent` | — (the system's) | — |
| openas 0.2 + this draft | §1 | §2 | journal, history | §3 | §4 | §5.2 | §5.1 | 0.2 §5 | 0.2 §3 |

So the pieces exist apart (App Intents' undo and confirmation, AG-UI's
interrupts and activity, MCP's elicitation); none has the transaction and the
preview, and none says what happens when a person and an agent act on one
thing at once. That is what this draft adds, under any of them as transport.

Sources: modelcontextprotocol.io/specification (2026-07-28, tools);
webmachinelearning.github.io/webmcp (Draft CG Report, 2 November 2026);
docs.ag-ui.com/concepts/events; a2ui.org/specification (v1.0 candidate);
developer.apple.com App Intents (`requestConfirmation`, `UndoableIntent`,
WWDC25 "Explore new advances in App Intents").

## 7. Open questions

- Previews across systems: an agent previewing steps in two systems at once
  has no joint "restored"; is a cross-system preview anything but each
  system's?
- The cost of a preview: a system MAY refuse previews over a budget (steps,
  time) — the same budget words as the real-time draft?
- Presence for actors without a place: an agent working on data rather than an
  interface — is its presence the records it reads?
