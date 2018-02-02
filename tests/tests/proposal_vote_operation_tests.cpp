#include <boost/test/unit_test.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <scorum/protocol/exceptions.hpp>

#include <scorum/protocol/types.hpp>
#include <scorum/chain/services/proposal.hpp>
#include <scorum/chain/evaluators/proposal_vote_evaluator.hpp>
#include <scorum/chain/schema/proposal_object.hpp>

#include "defines.hpp"

namespace tests {

using scorum::protocol::account_name_type;
using scorum::chain::proposal_object;
using scorum::chain::proposal_vote_operation;
using scorum::protocol::proposal_action;
using scorum::chain::proposal_id_type;
using scorum::chain::registration_pool_object;

class account_service_mock
{
public:
    void check_account_existence(const account_name_type& a)
    {
        checked_accounts.push_back(a);
    }

    std::vector<account_name_type> checked_accounts;
};

class proposal_service_mock
{
public:
    using action_t = scorum::protocol::proposal_action;

    proposal_service_mock()
    {
    }

    void remove(const proposal_object& proposal)
    {
        removed_proposals.push_back(proposal.id);
    }

    bool is_exists(const uint64_t id)
    {
        for (size_t i = 0; i < proposals.size(); ++i)
        {
            if (proposals[i].id == id)
                return true;
        }
        return false;
    }

    const proposal_object& get(const uint64_t id)
    {
        for (proposal_object& p : proposals)
        {
            if (p.id == id)
                return p;
        }

        BOOST_THROW_EXCEPTION(std::out_of_range("no such proposal"));
    }

    void vote_for(const account_name_type& account, const proposal_object& proposal)
    {
        voted_proposal.push_back(proposal.id);
        voters.insert(account);
    }

    size_t get_votes(const proposal_object&)
    {
        return voted;
    }

    bool is_expired(const proposal_object& proposal)
    {
        for (proposal_id_type id : expired)
        {
            if (id == proposal.id)
                return true;
        }

        return false;
    }

    void for_all_proposals_remove_from_voting_list(const account_name_type&)
    {
    }

    std::vector<proposal_object::cref_type> get_proposals()
    {
        std::vector<proposal_object::cref_type> p;
        return p;
    }

    std::vector<proposal_object> proposals;

    std::vector<proposal_id_type> removed_proposals;
    std::vector<proposal_id_type> voted_proposal;
    std::vector<proposal_id_type> expired;
    std::set<account_name_type> voters;

    size_t voted = 0;
};

class committee_service_mock
{
public:
    void add_member(const account_name_type& account)
    {
        added_members.push_back(account);
    }

    void exclude_member(const account_name_type& account)
    {
        excluded_members.push_back(account);
    }

    bool is_exists(const account_name_type& account) const
    {
        return existent_accounts.count(account) == 1 ? true : false;
    }

    uint64_t quorum_votes(uint64_t quorum_percent)
    {
        this->quorum_percent = quorum_percent;

        BOOST_REQUIRE(needed_votes > 0);
        return needed_votes;
    }

    std::set<account_name_type> existent_accounts;

    std::vector<account_name_type> added_members;
    std::vector<account_name_type> excluded_members;

    uint64_t needed_votes = 0;
    uint64_t quorum_percent = 0;
};

class properties_service_mock
{
public:
    void set_invite_quorum(uint64_t quorum)
    {
        new_invite_quorum = quorum;
    }

    void set_dropout_quorum(uint64_t quorum)
    {
        new_dropout_quorum = quorum;
    }

    void set_quorum(uint64_t quorum)
    {
        new_change_quorum = quorum;
    }

    uint64_t new_invite_quorum = 0;
    uint64_t new_dropout_quorum = 0;
    uint64_t new_change_quorum = 0;
};

typedef scorum::chain::proposal_vote_evaluator_t<account_service_mock,
                                                 proposal_service_mock,
                                                 committee_service_mock,
                                                 properties_service_mock>
    evaluator_test_impl;

class evaluator_mocked : public evaluator_test_impl
{
public:
    evaluator_mocked(account_service_mock& account_service,
                     proposal_service_mock& proposal_service,
                     committee_service_mock& committee_service,
                     properties_service_mock& properties_service)
        : evaluator_test_impl(account_service, proposal_service, committee_service, properties_service)
    {
    }

    void update_proposals_voting_list_and_execute()
    {
        evaluator_test_impl::update_proposals_voting_list_and_execute();
    }

    void execute_proposal(const proposal_object& proposal)
    {
        evaluator_test_impl::execute_proposal(proposal);
    }
};

class proposal_vote_evaluator_fixture
{
public:
    proposal_vote_evaluator_fixture()
        : evaluator(account_service, proposal_service, committee_service, properties_service)
    {
    }

    ~proposal_vote_evaluator_fixture()
    {
    }

    void apply()
    {
        evaluator.do_apply(op);
    }

    proposal_object& create_committee_proposal(proposal_action action = proposal_action::invite)
    {
        auto proposal = INIT_OBJ(proposal_object);
        proposal.creator = "alice";
        proposal.data = fc::variant("bob").as_string();
        proposal.action = action;

        proposal.id = proposal_service.proposals.size() + 1;
        proposal_service.proposals.push_back(proposal);

        committee_service.existent_accounts.insert(proposal.creator);
        committee_service.existent_accounts.insert("bob");

        op.voting_account = proposal.creator;
        op.proposal_id = proposal.id._id;

        return proposal_service.proposals[proposal_service.proposals.size() - 1];
    }

    proposal_object& create_quorum_change_proposal(uint64_t quorum, proposal_action action)
    {
        auto proposal = INIT_OBJ(proposal_object);
        proposal.creator = "alice";
        proposal.data = fc::variant(quorum).as_uint64();
        proposal.action = action;

        proposal.id = proposal_service.proposals.size() + 1;
        proposal_service.proposals.push_back(proposal);

        committee_service.existent_accounts.insert(proposal.creator);

        op.voting_account = proposal.creator;
        op.proposal_id = proposal.id._id;

        return proposal_service.proposals[proposal_service.proposals.size() - 1];
    }

    void configure_quorum()
    {
        proposal_service.voted = 1;
        committee_service.needed_votes = 1;
    }

    void configure_not_enough_quorum()
    {
        proposal_service.voted = 1;
        committee_service.needed_votes = 2;
    }

    proposal_vote_operation op;

    account_service_mock account_service;
    proposal_service_mock proposal_service;
    committee_service_mock committee_service;
    properties_service_mock properties_service;

    evaluator_mocked evaluator;
};

BOOST_FIXTURE_TEST_SUITE(proposal_vote_evaluator_tests, proposal_vote_evaluator_fixture)

SCORUM_TEST_CASE(throw_when_creator_is_not_in_committee)
{
    create_committee_proposal();

    committee_service.existent_accounts.erase(committee_service.existent_accounts.find("alice"));

    SCORUM_CHECK_EXCEPTION(apply(), fc::exception, "Account \"alice\" is not in committee.");
}

SCORUM_TEST_CASE(throw_when_proposal_does_not_exists)
{
    create_committee_proposal();

    op.proposal_id = 100;

    SCORUM_CHECK_EXCEPTION(apply(), fc::exception, "There is no proposal with id '100'");
}

SCORUM_TEST_CASE(throw_when_account_already_voted)
{
    proposal_object& p = create_committee_proposal();

    p.voted_accounts.insert(op.voting_account);

    SCORUM_CHECK_EXCEPTION(apply(), fc::exception, "Account \"alice\" already voted");
}

SCORUM_TEST_CASE(throw_exception_if_proposal_expired)
{
    auto p = create_committee_proposal();

    proposal_service.expired.push_back(p.id);

    BOOST_CHECK_THROW(apply(), fc::exception);

    SCORUM_CHECK_EXCEPTION(apply(), fc::exception, "Proposal '1' is expired.");
}

SCORUM_TEST_CASE(check_voter_name)
{
    auto p = create_committee_proposal(proposal_action::dropout);

    configure_quorum();

    apply();

    BOOST_CHECK(proposal_service.voters.count(op.voting_account) == 1);
}

SCORUM_TEST_CASE(check_account_existence_for_voter_name)
{
    auto p = create_committee_proposal(proposal_action::invite);
    configure_quorum();
    apply();

    BOOST_CHECK_EQUAL(account_service.checked_accounts[0], "alice");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(proposal_execute_tests, proposal_vote_evaluator_fixture)

SCORUM_TEST_CASE(dont_add_member_if_not_enough_quorum)
{
    auto p = create_committee_proposal(proposal_action::invite);

    configure_not_enough_quorum();

    evaluator.execute_proposal(p);

    BOOST_REQUIRE_EQUAL(0u, committee_service.added_members.size());
}

SCORUM_TEST_CASE(dont_dropout_if_not_enough_quorum)
{
    auto p = create_committee_proposal(proposal_action::dropout);

    configure_not_enough_quorum();

    evaluator.execute_proposal(p);

    BOOST_REQUIRE_EQUAL(0u, committee_service.excluded_members.size());
}

SCORUM_TEST_CASE(dont_remove_members_during_adding)
{
    auto p = create_committee_proposal(proposal_action::invite);

    configure_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(0u, committee_service.excluded_members.size());

    BOOST_REQUIRE_EQUAL(1u, committee_service.added_members.size());
    BOOST_CHECK_EQUAL(committee_service.added_members.front(), "bob");
}

SCORUM_TEST_CASE(dont_add_members_during_droping)
{
    auto p = create_committee_proposal(proposal_action::dropout);

    configure_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(0u, committee_service.added_members.size());

    BOOST_REQUIRE_EQUAL(1u, committee_service.excluded_members.size());
    BOOST_CHECK_EQUAL(committee_service.excluded_members.front(), "bob");
}

SCORUM_TEST_CASE(proposal_removed_after_droping_out_member)
{
    auto p = create_committee_proposal(proposal_action::dropout);

    configure_quorum();

    evaluator.execute_proposal(p);

    BOOST_REQUIRE_EQUAL(1u, proposal_service.removed_proposals.size());
    BOOST_CHECK_EQUAL(proposal_service.removed_proposals.front()._id, op.proposal_id);
}

SCORUM_TEST_CASE(change_invite_quorum)
{
    auto p = create_quorum_change_proposal(60, proposal_action::change_invite_quorum);

    configure_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(60u, properties_service.new_invite_quorum);
}

SCORUM_TEST_CASE(change_dropout_quorum)
{
    auto p = create_quorum_change_proposal(60, proposal_action::change_dropout_quorum);

    configure_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(60u, properties_service.new_dropout_quorum);
}

SCORUM_TEST_CASE(change_quorum)
{
    auto p = create_quorum_change_proposal(60, proposal_action::change_quorum);

    configure_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(60u, properties_service.new_change_quorum);
}

SCORUM_TEST_CASE(dont_change_invite_quorum)
{
    auto p = create_quorum_change_proposal(60, proposal_action::change_invite_quorum);

    configure_not_enough_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(0u, properties_service.new_invite_quorum);
}

SCORUM_TEST_CASE(dont_change_dropout_quorum)
{
    auto p = create_quorum_change_proposal(60, proposal_action::change_dropout_quorum);

    configure_not_enough_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(0u, properties_service.new_dropout_quorum);
}

SCORUM_TEST_CASE(dont_change_quorum)
{
    auto p = create_quorum_change_proposal(60, proposal_action::change_quorum);

    configure_not_enough_quorum();

    evaluator.execute_proposal(p);

    BOOST_CHECK_EQUAL(0u, properties_service.new_change_quorum);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_get_quorum)

BOOST_AUTO_TEST_CASE(sixty_percent_from_ten_is_six_votes)
{
    BOOST_CHECK_EQUAL(6u, scorum::chain::utils::get_quorum(10, 60));
}

BOOST_AUTO_TEST_CASE(sixty_percent_from_eight_is_four_votes)
{
    BOOST_CHECK_EQUAL(4u, scorum::chain::utils::get_quorum(8, 60));
}

BOOST_AUTO_TEST_CASE(sixty_percent_from_six_is_three_votes)
{
    BOOST_CHECK_EQUAL(3u, scorum::chain::utils::get_quorum(6, 60));
}

BOOST_AUTO_TEST_CASE(sixty_percent_from_five_is_three_votes)
{
    BOOST_CHECK_EQUAL(3u, scorum::chain::utils::get_quorum(5, 60));
}

BOOST_AUTO_TEST_CASE(sixty_percent_from_four_is_two_votes)
{
    BOOST_CHECK_EQUAL(2u, scorum::chain::utils::get_quorum(4, 60));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
