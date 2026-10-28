module openas;

import std;

namespace openas {

namespace {

constexpr std::array<std::string_view, 3> decisions{"run", "ask", "refuse"};

}  // namespace

bool Grant::covers(std::string_view verb) const {
    return std::ranges::any_of(scope, [&](const std::string& s) {
        if (s == "*") return true;
        if (s.ends_with(".*")) return verb.starts_with(std::string_view(s).substr(0, s.size() - 1));
        return s == verb;
    });
}

std::string format(const Grant& g) {
    std::string out = std::format("{}={}", format(g.actor), name(g.effect));
    if (g.scope.size() == 1 && g.scope[0] == "*") return out;
    for (std::size_t i = 0; i < g.scope.size(); ++i) {
        out += i ? "," : "@";
        out += g.scope[i];
    }
    return out;
}

std::optional<Grant> parse_grant(std::string_view text) {
    // The grant's '=' is the last one: an actor's `;via=` comes before it, and
    // an effect and a scope have none. A name may hold '@', an effect cannot.
    const auto eq = text.rfind('=');
    if (eq == std::string_view::npos) return std::nullopt;
    const auto rest = text.substr(eq + 1);
    const auto at = rest.find('@');
    auto actor = parse(text.substr(0, eq));
    auto effect = parse_effect(rest.substr(0, at));
    if (!actor || !effect) return std::nullopt;
    Grant g{std::move(*actor), *effect, {}};
    if (at == std::string_view::npos) {
        g.scope = {"*"};
        return g;
    }
    for (auto list = rest.substr(at + 1);;) {
        const auto comma = list.find(',');
        const auto one = list.substr(0, comma);
        if (one.empty()) return std::nullopt;
        g.scope.emplace_back(one);
        if (comma == std::string_view::npos) return g;
        list.remove_prefix(comma + 1);
    }
}

std::string_view name(Decision d) noexcept { return decisions[static_cast<std::size_t>(d)]; }

Decision decide(const Verb& verb, const Actor& actor, bool present, const Cap& cap, std::span<const Grant> grants) {
    if (present && actor.kind == Kind::person) return asks_first(verb.effect) ? Decision::ask : Decision::run;
    if (!cap.allows(verb.effect) || verb.person) return Decision::refuse;
    for (const Grant& g : grants)
        if (verb.effect <= g.effect && g.covers(verb.name) && acts_for(actor, g.actor)) return Decision::run;
    return asks_first(verb.effect) ? Decision::ask : Decision::run;
}

}  // namespace openas
