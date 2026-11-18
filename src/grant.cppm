// openas:grant — who may run what without asking, under a system's cap
// (SPEC.md §5), and the decision for one request.
export module openas:grant;

import std;
import :actor;
import :verb;

export namespace openas {

// The highest effect a verb may have and still run for an actor other than a
// present person; `none` lets no such actor run anything (§5.1).
struct Cap {
    std::optional<Effect> effect = Effect::external;  // nullopt: none

    [[nodiscard]] static Cap none() noexcept { return {std::nullopt}; }
    [[nodiscard]] bool allows(Effect e) const noexcept { return effect && e <= *effect; }
    bool operator==(const Cap&) const = default;
};

// `<actor>=<effect>[@<scope>,…]` (§5.5): verbs in scope, of an effect no
// higher than `effect`, run for `actor` and its delegates without asking.
struct Grant {
    Actor                    actor;
    Effect                   effect = Effect::read;
    std::vector<std::string> scope{"*"};  // verb names, `prefix.*`, or `*`

    [[nodiscard]] bool covers(std::string_view verb) const;
    bool operator==(const Grant&) const = default;
};

[[nodiscard]] std::string format(const Grant& g);
[[nodiscard]] std::optional<Grant> parse_grant(std::string_view text);

enum class Decision : std::uint8_t {
    run,     // runs without asking
    ask,     // asks first: the run ends with exit code 2 unless answered (§3.3)
    refuse,  // refused: error code `refused`, exit code 1
};

[[nodiscard]] std::string_view name(Decision d) noexcept;

// What a system does with a request for `verb` from `actor` (§5.3). `present`
// is the system's own observation that the actor is the person, at the
// machine (§2.7) — never what the request says.
[[nodiscard]] Decision decide(const Verb& verb, const Actor& actor, bool present, const Cap& cap,
                              std::span<const Grant> grants);

}  // namespace openas
