// openas:actor — an actor, the chain it acts for, and its text form (SPEC.md §2).
export module openas:actor;

import std;

export namespace openas {

enum class Kind : std::uint8_t { person, agent, program, service };

[[nodiscard]] std::string_view name(Kind k) noexcept;
[[nodiscard]] std::optional<Kind> parse_kind(std::string_view s) noexcept;

// An actor, and the chain it acts for: `via` names who delegated to it, the
// outermost first, each in short form (`person:alice`).
struct Actor {
    Kind                     kind = Kind::program;
    std::string              name;  // may be empty: "an agent", not which
    std::vector<std::string> via;

    bool operator==(const Actor&) const = default;
};

// How a record knows its actor (§2.6).
enum class Provenance : std::uint8_t { given, inherited, observed };

// `<kind>[:<name>][;via=<actor>,<actor>…]` (§2.4): the form an actor travels in.
[[nodiscard]] std::string format(const Actor& a);
// Reads the text form; keys it does not know are skipped.
[[nodiscard]] std::optional<Actor> parse(std::string_view text);
// `<kind>[:<name>]`, without the chain.
[[nodiscard]] std::string short_form(const Actor& a);

// `from` hands work to `kind:name`: the new actor acts for `from` and for
// everyone `from` acts for (§2.3).
[[nodiscard]] Actor delegate(const Actor& from, Kind kind, std::string name);

// `a` is `owner`, or acts for it.
[[nodiscard]] bool acts_for(const Actor& a, const Actor& owner);

}  // namespace openas
