module openas.conformance;

import std;
import openas;

namespace openas::conformance {

namespace {

bool lower(char c) { return c >= 'a' && c <= 'z'; }
bool digit(char c) { return c >= '0' && c <= '9'; }

bool verb_name(std::string_view n) {
    return !n.empty() && lower(n.front()) && std::ranges::all_of(n, [](char c) { return lower(c) || digit(c) || c == '-' || c == '.'; });
}

struct Holder {
    Report& r;
    void operator()(bool held, std::string what) const {
        if (held) ++r.held;
        else r.failed.push_back(std::move(what));
    }
};

}  // namespace

Report check(std::span<const Verb> verbs) {
    Report r;
    const Holder hold{r};
    std::set<std::string, std::less<>> names;
    for (const Verb& v : verbs) {
        hold(verb_name(v.name), std::format("7.1 {}: a verb's name is lower case words", v.name));
        hold(names.insert(v.name).second, std::format("7.2 {}: names are unique", v.name));
        hold(!v.summary.empty(), std::format("7.3 {}: says what it does", v.name));
        std::set<std::string, std::less<>> params;
        for (const Param& p : v.arguments) hold(params.insert(p.name).second, std::format("7.4 {}: {} is named once", v.name, p.name));
        for (const Param& p : v.options) hold(params.insert(p.name).second, std::format("7.4 {}: {} is named once", v.name, p.name));
        for (const std::string& a : v.answers)
            hold(std::ranges::any_of(v.options, [&](const Param& p) { return p.name == a; }),
                 std::format("7.5 {}: its answer {} is one of its options", v.name, a));
        if (asks_first(v.effect))
            hold(!v.answers.empty(),
                 std::format("7.6 {}: a verb that asks first ({}) names the option that answers", v.name, name(v.effect)));
    }
    return r;
}

Report check(const Actor& actor) {
    Report r;
    const std::string text = format(actor);
    Holder{r}(parse(text) == actor, std::format("7.7 {}: the text form reads back as the same actor", text));
    return r;
}

Report check(const Grant& grant) {
    Report r;
    const Holder hold{r};
    const std::string text = format(grant);
    hold(parse_grant(text) == grant, std::format("7.8 {}: the text form reads back as the same grant", text));
    for (const std::string& s : grant.scope) {
        const std::string_view name = s.ends_with(".*") ? std::string_view(s).substr(0, s.size() - 2) : std::string_view(s);
        hold(s == "*" || verb_name(name), std::format("7.8 {}: {} is a verb name or a prefix of one", text, s));
    }
    return r;
}

}  // namespace openas::conformance
