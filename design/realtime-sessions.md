# Real-time sessions, fairness and limits — a proposal for openas 0.3

Status: design draft, not normative. It proposes what openas 0.2 §8 leaves
open for systems where an actor plays or operates something that runs on its
own clock — a game, a simulation, a live instrument — and records what a first
implementation shows. Text that survives validation moves into SPEC.md; until
then nothing here is part of the standard.

Validation: the component breakout in github.com/Sunrisepeak/mui (branch
openmui-design, `showcase/widget-breakout`): an external agent, speaking openas
envelopes over the system's local endpoint, plays a whole game headless at a
person's tier, and replays to the same bytes.

## 1. Sessions on a clock

1.1 A *session* is an actor's sustained use of a system whose state advances
by itself in *steps* (a game's ticks, a simulation's frames). Every step has a
number; numbers only increase within a branch of play.

1.2 *Observe* is a `read` verb that answers what the actor is shown and the
step it shows (`step`). A system MAY also push observations as `data`
records of a long-running request (§6.1), one per step or per N steps.

1.3 *Act* is a verb whose request names the step it was decided on (`seen`).
The system answers the step at which the action takes effect (`at`), which is
never earlier than `seen` plus the actor's reaction (§2.2) nor earlier than
the next step to be played. An action naming a step not yet played is refused
(`invalid-argument`).

1.4 An action holds until the actor's next action: like a held key, not a
pulse. A system that wants a pulse says so in the verb.

1.5 Deadlines: a request MAY carry `deadline` (a step). An action that cannot
take effect by it is refused (`timeout`) rather than applied late.

1.6 A system that records play records each action with the step it took
effect, so a replay feeds the same actions at the same steps; the actor's
identity and tier (§2) are part of the record.

## 2. Tiers: what an actor sees and how fast it may act

2.1 A system that admits actors to play declares *tiers*. A tier has a name
and states:

| Field | Meaning |
|---|---|
| `view` | `drawn`: what a person is shown; `full`: everything, hidden state included |
| `reaction_ms` | the least time between the step an action saw and the step it takes effect |
| `actions_per_second` | at most this many actions a second of steps (0: no limit) |
| `operations` | the verbs the tier may use |

2.2 A tier named `human` SHOULD hold an actor to what a person could do: a
person's view, a person's reaction (100 ms is a common floor) and rate, and a
person's means of acting (the same input actions, not privileged verbs).

2.3 A system declares which tiers it allows, and the person configures which
of them are on; an actor uses exactly one tier per session, named in its
requests and in the record. An actor's tier is visible to other participants.

## 3. Budgets, rate limits and stopping

3.1 A *budget* bounds what an actor may spend — actions, steps, a currency of
the system — per session or per period. A request over its budget is refused
(`refused`), naming the budget.

3.2 Rate limits are budgets per time. An action over a tier's rate is refused,
not queued.

3.3 A present person MAY *stop* an actor: its session ends at the next step,
its pending actions are dropped, and further requests are refused until a new
grant. Stopping is recorded.

## 4. Grants over time, and several agents

4.1 A grant MAY carry an expiry (`until`, a time); after it the grant does not
apply (requests ask or are refused as §5.3 of 0.2 says). The text form gains
`;until=<RFC 3339 time>`.

4.2 Several agents in one system each act as themselves; a record names each
actor (with its `via` chain, §2.2 of 0.2), and a participant can learn which
other participants are agents and at which tier.

4.3 Delegation in a session follows §2.3 of 0.2: a helper an agent starts acts
within the agent's tier and grants, never beyond them.

## 5. What the first implementation shows

- Holding actions like keys (1.4) and stating the effect step (1.3) make
  play deterministic: the same submissions at the same steps give the same
  bytes; an external agent played breakout twice to one checksum.
- A person's tier is enforceable at the system without trusting the agent:
  view filtering (2.1), the reaction floor and the rate are applied where
  actions enter the simulation.
- Refusals carry the reason (too often, not yet played, players off), which
  the agent can act on.

Open: a pushed observation stream's back-pressure, sessions spanning
processes (a token), and how budgets are declared in a verb description.
