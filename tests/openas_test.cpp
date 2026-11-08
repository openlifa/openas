// openas: actors and their delegation, effects, grants and the decision for a
// request, the envelope's codes, and conformance — with a description that
// breaks each clause, so a check that holds nothing cannot pass.
import std;
import boost.ut;
import openas;
import openas.conformance;

using namespace boost::ut;

namespace {

bool failed(const openas::conformance::Report& r, std::string_view clause) {
    return std::ranges::any_of(r.failed, [&](const std::string& f) { return f.starts_with(clause); });
}

openas::Actor actor(std::string_view text) { return openas::parse(text).value(); }

}  // namespace

int main() {
    "an actor travels as text, and delegation only adds to its chain"_test = [] {
        const auto alice = actor("person:alice");
        expect(alice.kind == openas::Kind::person);
        const auto planner = openas::delegate(alice, openas::Kind::agent, "planner");
        expect(openas::format(planner) == "agent:planner;via=person:alice");
        const auto reader = openas::delegate(planner, openas::Kind::agent, "reader");
        expect(openas::format(reader) == "agent:reader;via=person:alice,agent:planner");
        expect(openas::parse(openas::format(reader)) == reader);
        expect(openas::acts_for(reader, alice));
        expect(openas::acts_for(reader, planner));
        expect(!openas::acts_for(planner, reader));
        expect(!openas::acts_for(alice, planner));
        expect(!openas::acts_for(reader, actor("person:bob")));
        expect(openas::format(actor("agent")) == "agent");
        expect(actor("person:alice@example.org").name == "alice@example.org");
    };

    "a key the reader does not know is skipped; via appears once"_test = [] {
        expect(openas::parse("agent:x;budget=3") == actor("agent:x"));
        expect(openas::parse("agent:x;budget=3;via=person:a") == actor("agent:x;via=person:a"));
        for (const auto* bad : {"", "robot:x", "agent:", "agent:a b", "agent:x;via=", "agent:x;via=person:a,", "agent:x;via=bogus",
                                "agent:x;via=person:a;via=person:b", "agent:x;budget", "agent:x;=3"})
            expect(!openas::parse(bad)) << bad;
    };

    "effects are ordered, the undoable ones ask first, and an unknown one is external"_test = [] {
        for (auto e : {openas::Effect::read, openas::Effect::reversible, openas::Effect::irreversible, openas::Effect::external})
            expect(openas::parse_effect(openas::name(e)) == e);
        expect(!openas::parse_effect("dangerous"));
        expect(openas::read_effect("dangerous") == openas::Effect::external);
        expect(!openas::asks_first(openas::Effect::read));
        expect(!openas::asks_first(openas::Effect::reversible));
        expect(openas::asks_first(openas::Effect::irreversible));
        expect(openas::asks_first(openas::Effect::external));
    };

    "a grant travels as text and covers its scope"_test = [] {
        const auto g = openas::parse_grant("agent:claude;via=person:alice=irreversible@notes.*,files.open").value();
        expect(g.actor == actor("agent:claude;via=person:alice"));
        expect(g.effect == openas::Effect::irreversible);
        expect(g.covers("notes.delete") && g.covers("files.open"));
        expect(!g.covers("files.delete") && !g.covers("notes"));
        expect(openas::format(g) == "agent:claude;via=person:alice=irreversible@notes.*,files.open");
        const auto all = openas::parse_grant("person:bo@x.org=read").value();
        expect(all.actor.name == "bo@x.org" && all.covers("anything"));
        expect(openas::format(all) == "person:bo@x.org=read");
        for (const auto* bad : {"agent:x", "agent:x=loud", "agent:x=read@", "agent:x=read@a,", "robot=read"})
            expect(!openas::parse_grant(bad)) << bad;
    };

    "a request is decided by presence, the cap, the person's own mark and grants"_test = [] {
        using D = openas::Decision;
        using E = openas::Effect;
        const openas::Verb open{.name = "files.open", .summary = "Open", .effect = E::read};
        const openas::Verb edit{.name = "notes.edit", .summary = "Edit", .effect = E::reversible};
        const openas::Verb drop{.name = "notes.delete", .summary = "Delete", .effect = E::irreversible};
        const openas::Verb pay{.name = "pay", .summary = "Pay", .effect = E::external, .person = true};
        const auto alice = actor("person:alice");
        const auto agent = openas::delegate(alice, openas::Kind::agent, "claude");
        const auto helper = openas::delegate(agent, openas::Kind::agent, "reader");
        const openas::Cap open_cap;
        const std::vector<openas::Grant> none;
        // Without grants: read and reversible run, the rest ask; the person's own is refused.
        expect(openas::decide(open, agent, false, open_cap, none) == D::run);
        expect(openas::decide(edit, agent, false, open_cap, none) == D::run);
        expect(openas::decide(drop, agent, false, open_cap, none) == D::ask);
        expect(openas::decide(pay, agent, false, open_cap, none) == D::refuse);
        // The present person runs their own verb (asking first, as §3.2 says);
        // the same person, named by a request, does not.
        expect(openas::decide(pay, alice, true, open_cap, none) == D::ask);
        expect(openas::decide(pay, alice, false, open_cap, none) == D::refuse);
        // A grant runs what it covers, for its actor and its delegates, and no more.
        const std::vector<openas::Grant> grants{openas::parse_grant("agent:claude;via=person:alice=irreversible@notes.*").value()};
        expect(openas::decide(drop, agent, false, open_cap, grants) == D::run);
        expect(openas::decide(drop, helper, false, open_cap, grants) == D::run);
        expect(openas::decide(drop, actor("agent:claude"), false, open_cap, grants) == D::ask);
        expect(openas::decide(pay, agent, false, open_cap, grants) == D::refuse);
        // The cap is above every grant: read-only refuses an edit even when granted.
        const openas::Cap read_only{openas::Effect::read};
        expect(openas::decide(edit, agent, false, read_only, grants) == D::refuse);
        expect(openas::decide(open, agent, false, read_only, grants) == D::run);
        expect(openas::decide(open, agent, false, openas::Cap::none(), grants) == D::refuse);
        expect(openas::decide(open, alice, true, openas::Cap::none(), none) == D::run);
    };

    "the envelope's kinds and codes"_test = [] {
        for (auto k : {openas::RecordKind::phase, openas::RecordKind::progress, openas::RecordKind::log, openas::RecordKind::notice,
                       openas::RecordKind::error, openas::RecordKind::data, openas::RecordKind::result})
            expect(openas::parse_record_kind(openas::name(k)) == k);
        expect(openas::read_error_code("stale") == openas::error_code::stale);
        expect(openas::read_error_code("melted") == openas::error_code::failed);
        expect(openas::exit_code::not_ready == 3);
    };

    "the check refuses each broken clause"_test = [] {
        using V = openas::Verb;
        const openas::Param yes{.name = "yes", .flag = true};
        const auto check = [](std::vector<V> vs) { return openas::conformance::check(std::span<const V>(vs)); };
        const V good{.name = "rm", .summary = "Remove", .effect = openas::Effect::irreversible, .options = {yes}, .answers = {"yes"}};
        expect(check({good}).ok());
        expect(failed(check({V{.name = "Remove", .summary = "x"}}), "7.1"));
        expect(failed(check({good, good}), "7.2"));
        expect(failed(check({V{.name = "ls"}}), "7.3"));
        expect(failed(check({V{.name = "ls", .summary = "x", .options = {yes, yes}}}), "7.4"));
        expect(failed(check({V{.name = "ls", .summary = "x", .answers = {"yes"}}}), "7.5"));
        expect(failed(check({V{.name = "rm", .summary = "x", .effect = openas::Effect::irreversible, .options = {yes}}}), "7.6"));
        expect(openas::conformance::check(actor("agent:x;via=person:a")).ok());
        expect(failed(openas::conformance::check(openas::Actor{openas::Kind::agent, "a b", {}}), "7.7"));
        expect(openas::conformance::check(openas::parse_grant("agent:x=read@notes.*").value()).ok());
        expect(failed(openas::conformance::check(openas::Grant{actor("agent:x"), openas::Effect::read, {"Notes"}}), "7.8"));
    };

    return static_cast<int>(cfg<>.run({.report_errors = true}));
}
