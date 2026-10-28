module openas;

import std;

namespace openas {

namespace {

constexpr std::array<std::string_view, 4> kinds{"person", "agent", "program", "service"};

bool name_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
           c == '@';
}

std::optional<Actor> parse_short(std::string_view text) {
    const auto colon = text.find(':');
    const auto kind = parse_kind(text.substr(0, colon));
    if (!kind) return std::nullopt;
    Actor a{*kind, {}, {}};
    if (colon != std::string_view::npos) {
        a.name = std::string(text.substr(colon + 1));
        if (a.name.empty() || !std::ranges::all_of(a.name, name_char)) return std::nullopt;
    }
    return a;
}

}  // namespace

std::string_view name(Kind k) noexcept { return kinds[static_cast<std::size_t>(k)]; }

std::optional<Kind> parse_kind(std::string_view s) noexcept {
    for (std::size_t i = 0; i < kinds.size(); ++i)
        if (kinds[i] == s) return static_cast<Kind>(i);
    return std::nullopt;
}

std::string short_form(const Actor& a) {
    return a.name.empty() ? std::string(name(a.kind)) : std::format("{}:{}", name(a.kind), a.name);
}

std::string format(const Actor& a) {
    std::string out = short_form(a);
    for (std::size_t i = 0; i < a.via.size(); ++i) {
        out += i ? "," : ";via=";
        out += a.via[i];
    }
    return out;
}

std::optional<Actor> parse(std::string_view text) {
    auto a = parse_short(text.substr(0, text.find(';')));
    if (!a) return std::nullopt;
    bool via = false;
    for (auto rest = text.substr(std::min(text.size(), text.find(';'))); !rest.empty();) {
        rest.remove_prefix(1);  // ';'
        const auto item = rest.substr(0, rest.find(';'));
        rest.remove_prefix(item.size());
        const auto eq = item.find('=');
        if (eq == std::string_view::npos || eq == 0) return std::nullopt;
        if (item.substr(0, eq) != "via") continue;  // reserved for later versions (§2.4)
        if (via) return std::nullopt;               // at most once
        via = true;
        // One or more actors, comma-separated: never an empty one.
        for (auto list = item.substr(eq + 1);;) {
            const auto comma = list.find(',');
            const auto one = list.substr(0, comma);
            if (!parse_short(one)) return std::nullopt;
            a->via.emplace_back(one);
            if (comma == std::string_view::npos) break;
            list.remove_prefix(comma + 1);
        }
    }
    return a;
}

Actor delegate(const Actor& from, Kind kind, std::string name) {
    Actor to{kind, std::move(name), from.via};
    to.via.push_back(short_form(from));
    return to;
}

bool acts_for(const Actor& a, const Actor& owner) {
    if (a == owner) return true;
    const auto& chain = owner.via;
    if (a.via.size() < chain.size() + 1) return false;
    for (std::size_t i = 0; i < chain.size(); ++i)
        if (a.via[i] != chain[i]) return false;
    return a.via[chain.size()] == short_form(owner);
}

}  // namespace openas
