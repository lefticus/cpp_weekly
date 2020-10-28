#include <string_view>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <memory_resource>
#include <benchmark/benchmark.h>
#include <array>
#include <unordered_set>

constexpr const char *long_strings[] = { "Am_a_long_word_string", "terminated_a_long_word_string", "it_a_long_word_string", "excellence_a_long_word_string", "invitation_a_long_word_string", "projection_a_long_word_string", "as_a_long_word_string", "She_a_long_word_string", "graceful_a_long_word_string", "shy_a_long_word_string", "believed_a_long_word_string", "distance_a_long_word_string", "use_a_long_word_string", "nay_a_long_word_string", "Lively_a_long_word_string", "is_a_long_word_string", "people_a_long_word_string", "so_a_long_word_string", "basket_a_long_word_string", "ladies_a_long_word_string", "window_a_long_word_string", "expect_a_long_word_string", "Supply_a_long_word_string", "as_a_long_word_string", "so_a_long_word_string", "period_a_long_word_string", "it_a_long_word_string", "enough_a_long_word_string", "income_a_long_word_string", "he_a_long_word_string", "genius_a_long_word_string", "Themselves_a_long_word_string", "acceptance_a_long_word_string", "bed_a_long_word_string", "sympathize_a_long_word_string", "get_a_long_word_string", "dissimilar_a_long_word_string", "way_a_long_word_string", "admiration_a_long_word_string", "son_a_long_word_string", "Design_a_long_word_string", "for_a_long_word_string", "are_a_long_word_string", "edward_a_long_word_string", "regret_a_long_word_string", "met_a_long_word_string", "lovers_a_long_word_string", "This_a_long_word_string", "are_a_long_word_string", "calm_a_long_word_string", "case_a_long_word_string", "roof_a_long_word_string", "and_a_long_word_string", "Needed_a_long_word_string", "feebly_a_long_word_string", "dining_a_long_word_string", "oh_a_long_word_string", "talked_a_long_word_string", "wisdom_a_long_word_string", "oppose_a_long_word_string", "at_a_long_word_string", "Applauded_a_long_word_string", "use_a_long_word_string", "attempted_a_long_word_string", "strangers_a_long_word_string", "now_a_long_word_string", "are_a_long_word_string", "middleton_a_long_word_string", "concluded_a_long_word_string", "had_a_long_word_string", "It_a_long_word_string", "is_a_long_word_string", "tried_a_long_word_string", "﻿no_a_long_word_string", "added_a_long_word_string", "purse_a_long_word_string", "shall_a_long_word_string", "no_a_long_word_string", "on_a_long_word_string", "truth_a_long_word_string", "Pleased_a_long_word_string", "anxious_a_long_word_string", "or_a_long_word_string", "as_a_long_word_string", "in_a_long_word_string", "by_a_long_word_string", "viewing_a_long_word_string", "forbade_a_long_word_string", "minutes_a_long_word_string", "prevent_a_long_word_string", "Too_a_long_word_string", "leave_a_long_word_string", "had_a_long_word_string", "those_a_long_word_string", "get_a_long_word_string", "being_a_long_word_string", "led_a_long_word_string", "weeks_a_long_word_string", "blind_a_long_word_string", "Had_a_long_word_string", "men_a_long_word_string", "rose_a_long_word_string", "from_a_long_word_string", "down_a_long_word_string", "lady_a_long_word_string", "able_a_long_word_string", "Its_a_long_word_string", "son_a_long_word_string", "him_a_long_word_string", "ferrars_a_long_word_string", "proceed_a_long_word_string", "six_a_long_word_string", "parlors_a_long_word_string", "Her_a_long_word_string", "say_a_long_word_string", "projection_a_long_word_string", "age_a_long_word_string", "announcing_a_long_word_string", "decisively_a_long_word_string", "men_a_long_word_string", "Few_a_long_word_string", "gay_a_long_word_string", "sir_a_long_word_string", "those_a_long_word_string", "green_a_long_word_string", "men_a_long_word_string", "timed_a_long_word_string", "downs_a_long_word_string", "widow_a_long_word_string", "chief_a_long_word_string", "Prevailed_a_long_word_string", "remainder_a_long_word_string", "may_a_long_word_string", "propriety_a_long_word_string", "can_a_long_word_string", "and_a_long_word_string", "And_a_long_word_string", "sir_a_long_word_string", "dare_a_long_word_string", "view_a_long_word_string", "but_a_long_word_string", "over_a_long_word_string", "man_a_long_word_string", "So_a_long_word_string", "at_a_long_word_string", "within_a_long_word_string", "mr_a_long_word_string", "to_a_long_word_string", "simple_a_long_word_string", "assure_a_long_word_string", "Mr_a_long_word_string", "disposing_a_long_word_string", "continued_a_long_word_string", "it_a_long_word_string", "offending_a_long_word_string", "arranging_a_long_word_string", "in_a_long_word_string", "we_a_long_word_string", "Extremity_a_long_word_string", "as_a_long_word_string", "if_a_long_word_string", "breakfast_a_long_word_string", "agreement_a_long_word_string", "Off_a_long_word_string", "now_a_long_word_string", "mistress_a_long_word_string", "provided_a_long_word_string", "out_a_long_word_string", "horrible_a_long_word_string", "opinions_a_long_word_string", "Prevailed_a_long_word_string", "mr_a_long_word_string", "tolerably_a_long_word_string", "discourse_a_long_word_string", "assurance_a_long_word_string", "estimable_a_long_word_string", "applauded_a_long_word_string", "to_a_long_word_string", "so_a_long_word_string", "Him_a_long_word_string", "everything_a_long_word_string", "melancholy_a_long_word_string", "uncommonly_a_long_word_string", "but_a_long_word_string", "solicitude_a_long_word_string", "inhabiting_a_long_word_string", "projection_a_long_word_string", "off_a_long_word_string", "Connection_a_long_word_string", "stimulated_a_long_word_string", "estimating_a_long_word_string", "excellence_a_long_word_string", "an_a_long_word_string", "to_a_long_word_string", "impression_a_long_word_string", "For_a_long_word_string", "norland_a_long_word_string", "produce_a_long_word_string", "age_a_long_word_string", "wishing_a_long_word_string", "To_a_long_word_string", "figure_a_long_word_string", "on_a_long_word_string", "it_a_long_word_string", "spring_a_long_word_string", "season_a_long_word_string", "up_a_long_word_string", "Her_a_long_word_string", "provision_a_long_word_string", "acuteness_a_long_word_string", "had_a_long_word_string", "excellent_a_long_word_string", "two_a_long_word_string", "why_a_long_word_string", "intention_a_long_word_string", "As_a_long_word_string", "called_a_long_word_string", "mr_a_long_word_string", "needed_a_long_word_string", "praise_a_long_word_string", "at_a_long_word_string", "Assistance_a_long_word_string", "imprudence_a_long_word_string", "yet_a_long_word_string", "sentiments_a_long_word_string", "unpleasant_a_long_word_string", "expression_a_long_word_string", "met_a_long_word_string", "surrounded_a_long_word_string", "not_a_long_word_string", "Be_a_long_word_string", "at_a_long_word_string", "talked_a_long_word_string", "ye_a_long_word_string", "though_a_long_word_string", "secure_a_long_word_string", "nearer_a_long_word_string", "Rooms_a_long_word_string", "oh_a_long_word_string", "fully_a_long_word_string", "taken_a_long_word_string", "by_a_long_word_string", "worse_a_long_word_string", "do_a_long_word_string", "Points_a_long_word_string", "afraid_a_long_word_string", "but_a_long_word_string", "may_a_long_word_string", "end_a_long_word_string", "law_a_long_word_string", "lasted_a_long_word_string", "Was_a_long_word_string", "out_a_long_word_string", "laughter_a_long_word_string", "raptures_a_long_word_string", "returned_a_long_word_string", "outweigh_a_long_word_string", "Luckily_a_long_word_string", "cheered_a_long_word_string", "colonel_a_long_word_string", "me_a_long_word_string", "do_a_long_word_string", "we_a_long_word_string", "attacks_a_long_word_string", "on_a_long_word_string", "highest_a_long_word_string", "enabled_a_long_word_string", "Tried_a_long_word_string", "law_a_long_word_string", "yet_a_long_word_string", "style_a_long_word_string", "child_a_long_word_string", "Bore_a_long_word_string", "of_a_long_word_string", "true_a_long_word_string", "of_a_long_word_string", "no_a_long_word_string", "be_a_long_word_string", "deal_a_long_word_string", "Frequently_a_long_word_string", "sufficient_a_long_word_string", "in_a_long_word_string", "be_a_long_word_string", "unaffected_a_long_word_string", "The_a_long_word_string", "furnished_a_long_word_string", "she_a_long_word_string", "concluded_a_long_word_string", "depending_a_long_word_string", "procuring_a_long_word_string", "concealed_a_long_word_string", "Game_a_long_word_string", "of_a_long_word_string", "as_a_long_word_string", "rest_a_long_word_string", "time_a_long_word_string", "eyes_a_long_word_string", "with_a_long_word_string", "of_a_long_word_string", "this_a_long_word_string", "it_a_long_word_string", "Add_a_long_word_string", "was_a_long_word_string", "music_a_long_word_string", "merry_a_long_word_string", "any_a_long_word_string", "truth_a_long_word_string", "since_a_long_word_string", "going_a_long_word_string", "Happiness_a_long_word_string", "she_a_long_word_string", "ham_a_long_word_string", "but_a_long_word_string", "instantly_a_long_word_string", "put_a_long_word_string", "departure_a_long_word_string", "propriety_a_long_word_string", "She_a_long_word_string", "amiable_a_long_word_string", "all_a_long_word_string", "without_a_long_word_string", "say_a_long_word_string", "spirits_a_long_word_string", "shy_a_long_word_string", "clothes_a_long_word_string", "morning_a_long_word_string", "Frankness_a_long_word_string", "in_a_long_word_string", "extensive_a_long_word_string", "to_a_long_word_string", "belonging_a_long_word_string", "improving_a_long_word_string", "so_a_long_word_string", "certainty_a_long_word_string", "Resolution_a_long_word_string", "devonshire_a_long_word_string", "pianoforte_a_long_word_string", "assistance_a_long_word_string", "an_a_long_word_string", "he_a_long_word_string", "particular_a_long_word_string", "middletons_a_long_word_string", "is_a_long_word_string", "of_a_long_word_string", "Explain_a_long_word_string", "ten_a_long_word_string", "man_a_long_word_string", "uncivil_a_long_word_string", "engaged_a_long_word_string", "conduct_a_long_word_string", "Am_a_long_word_string", "likewise_a_long_word_string", "betrayed_a_long_word_string", "as_a_long_word_string", "declared_a_long_word_string", "absolute_a_long_word_string", "do_a_long_word_string", "Taste_a_long_word_string", "oh_a_long_word_string", "spoke_a_long_word_string", "about_a_long_word_string", "no_a_long_word_string", "solid_a_long_word_string", "of_a_long_word_string", "hills_a_long_word_string", "up_a_long_word_string", "shade_a_long_word_string", "Occasion_a_long_word_string", "so_a_long_word_string", "bachelor_a_long_word_string", "humoured_a_long_word_string", "striking_a_long_word_string", "by_a_long_word_string", "attended_a_long_word_string", "doubtful_a_long_word_string", "be_a_long_word_string", "it_a_long_word_string", "Of_a_long_word_string", "friendship_a_long_word_string", "on_a_long_word_string", "inhabiting_a_long_word_string", "diminution_a_long_word_string", "discovered_a_long_word_string", "as_a_long_word_string", "Did_a_long_word_string", "friendly_a_long_word_string", "eat_a_long_word_string", "breeding_a_long_word_string", "building_a_long_word_string", "few_a_long_word_string", "nor_a_long_word_string", "Object_a_long_word_string", "he_a_long_word_string", "barton_a_long_word_string", "no_a_long_word_string", "effect_a_long_word_string", "played_a_long_word_string", "valley_a_long_word_string", "afford_a_long_word_string", "Period_a_long_word_string", "so_a_long_word_string", "to_a_long_word_string", "oppose_a_long_word_string", "we_a_long_word_string", "little_a_long_word_string", "seeing_a_long_word_string", "or_a_long_word_string", "branch_a_long_word_string", "Announcing_a_long_word_string", "contrasted_a_long_word_string", "not_a_long_word_string", "imprudence_a_long_word_string", "add_a_long_word_string", "frequently_a_long_word_string", "you_a_long_word_string", "possession_a_long_word_string", "mrs_a_long_word_string", "Period_a_long_word_string", "saw_a_long_word_string", "his_a_long_word_string", "houses_a_long_word_string", "square_a_long_word_string", "and_a_long_word_string", "misery_a_long_word_string", "Hour_a_long_word_string", "had_a_long_word_string", "held_a_long_word_string", "lain_a_long_word_string", "give_a_long_word_string", "yet_a_long_word_string", "In_a_long_word_string", "up_a_long_word_string", "so_a_long_word_string", "discovery_a_long_word_string", "my_a_long_word_string", "middleton_a_long_word_string", "eagerness_a_long_word_string", "dejection_a_long_word_string", "explained_a_long_word_string", "Estimating_a_long_word_string", "excellence_a_long_word_string", "ye_a_long_word_string", "contrasted_a_long_word_string", "insensible_a_long_word_string", "as_a_long_word_string", "Oh_a_long_word_string", "up_a_long_word_string", "unsatiable_a_long_word_string", "advantages_a_long_word_string", "decisively_a_long_word_string", "as_a_long_word_string", "at_a_long_word_string", "interested_a_long_word_string", "Present_a_long_word_string", "suppose_a_long_word_string", "in_a_long_word_string", "esteems_a_long_word_string", "in_a_long_word_string", "demesne_a_long_word_string", "colonel_a_long_word_string", "it_a_long_word_string", "to_a_long_word_string", "End_a_long_word_string", "horrible_a_long_word_string", "she_a_long_word_string", "landlord_a_long_word_string", "screened_a_long_word_string", "stanhill_a_long_word_string", "Repeated_a_long_word_string", "offended_a_long_word_string", "you_a_long_word_string", "opinions_a_long_word_string", "off_a_long_word_string", "dissuade_a_long_word_string", "ask_a_long_word_string", "packages_a_long_word_string", "screened_a_long_word_string", "She_a_long_word_string", "alteration_a_long_word_string", "everything_a_long_word_string", "sympathize_a_long_word_string", "impossible_a_long_word_string", "his_a_long_word_string", "get_a_long_word_string", "compliment_a_long_word_string", "Collected_a_long_word_string", "few_a_long_word_string", "extremity_a_long_word_string", "suffering_a_long_word_string", "met_a_long_word_string", "had_a_long_word_string", "sportsman_a_long_word_string", "Mind_a_long_word_string", "what_a_long_word_string", "no_a_long_word_string", "by_a_long_word_string", "kept_a_long_word_string", "Celebrated_a_long_word_string", "no_a_long_word_string", "he_a_long_word_string", "decisively_a_long_word_string", "thoroughly_a_long_word_string", "Our_a_long_word_string", "asked_a_long_word_string", "point_a_long_word_string", "her_a_long_word_string", "she_a_long_word_string", "seems_a_long_word_string", "New_a_long_word_string", "plenty_a_long_word_string", "she_a_long_word_string", "horses_a_long_word_string", "parish_a_long_word_string", "design_a_long_word_string", "you_a_long_word_string", "Stuff_a_long_word_string", "sight_a_long_word_string", "equal_a_long_word_string", "of_a_long_word_string", "my_a_long_word_string", "woody_a_long_word_string", "Him_a_long_word_string", "children_a_long_word_string", "bringing_a_long_word_string", "goodness_a_long_word_string", "suitable_a_long_word_string", "she_a_long_word_string", "entirely_a_long_word_string", "put_a_long_word_string", "far_a_long_word_string", "daughter_a_long_word_string", "She_a_long_word_string", "wholly_a_long_word_string", "fat_a_long_word_string", "who_a_long_word_string", "window_a_long_word_string", "extent_a_long_word_string", "either_a_long_word_string", "formal_a_long_word_string", "Removing_a_long_word_string", "welcomed_a_long_word_string", "civility_a_long_word_string", "or_a_long_word_string", "hastened_a_long_word_string", "is_a_long_word_string", "Justice_a_long_word_string", "elderly_a_long_word_string", "but_a_long_word_string", "perhaps_a_long_word_string", "expense_a_long_word_string", "six_a_long_word_string", "her_a_long_word_string", "are_a_long_word_string", "another_a_long_word_string", "passage_a_long_word_string", "Full_a_long_word_string", "her_a_long_word_string", "ten_a_long_word_string", "open_a_long_word_string", "fond_a_long_word_string", "walk_a_long_word_string", "not_a_long_word_string", "down_a_long_word_string", "For_a_long_word_string", "request_a_long_word_string", "general_a_long_word_string", "express_a_long_word_string", "unknown_a_long_word_string", "are_a_long_word_string", "He_a_long_word_string", "in_a_long_word_string", "just_a_long_word_string", "mr_a_long_word_string", "door_a_long_word_string", "body_a_long_word_string", "held_a_long_word_string", "john_a_long_word_string", "down_a_long_word_string", "he_a_long_word_string", "So_a_long_word_string", "journey_a_long_word_string", "greatly_a_long_word_string", "or_a_long_word_string", "garrets_a_long_word_string", "Draw_a_long_word_string", "door_a_long_word_string", "kept_a_long_word_string", "do_a_long_word_string", "so_a_long_word_string", "come_a_long_word_string", "on_a_long_word_string", "open_a_long_word_string", "mean_a_long_word_string", "Estimating_a_long_word_string", "stimulated_a_long_word_string", "how_a_long_word_string", "reasonably_a_long_word_string", "precaution_a_long_word_string", "diminution_a_long_word_string", "she_a_long_word_string", "simplicity_a_long_word_string", "sir_a_long_word_string", "but_a_long_word_string", "Questions_a_long_word_string", "am_a_long_word_string", "sincerity_a_long_word_string", "zealously_a_long_word_string", "concluded_a_long_word_string", "consisted_a_long_word_string", "or_a_long_word_string", "no_a_long_word_string", "gentleman_a_long_word_string", "it" };

template<class Output, class T, std::size_t N, std::size_t... I>
constexpr std::array<Output, N>
  to_array_impl(T (&a)[N], std::index_sequence<I...>)
{
  return { { a[I]... } };
}

template<class Output, class T, std::size_t N>
constexpr std::array<Output, N> to_array(T (&a)[N])
{
  return to_array_impl<Output>(a, std::make_index_sequence<N>{});
}

constexpr auto long_string_views = to_array<std::string_view>(long_strings);

/*
static void Alpha(benchmark::State& state) {
  int total = 0;
//  std::array<std::uint8_t, 10000> buffer{};
//  std::pmr::monotonic_buffer_resource mem_resource{buffer.long_strings(), buffer.size()};
  //std::pmr::unsynchronized_pool_resource mem_resource;
  //std::pmr::monotonic_buffer_resource mem_resource;
  std::pmr::monotonic_buffer_resource mem_resource(sizeof(std::pmr::string) * 1000);
  std::pmr::polymorphic_allocator alloc{&mem_resource};
  auto *obj = alloc.new_object<std::pmr::set<std::string_view>>(std::begin(long_strings), std::end(long_strings));
  auto &words = *obj;
    
  for (auto _ : state) {
    for (const auto &key : words)
    {
      total += key.size();
    }
    benchmark::DoNotOptimize(total);
  }
}
BENCHMARK(Alpha);

static void Alpha2(benchmark::State& state) {
  int total = 0;
  auto words = std::set<std::string_view>(std::begin(long_strings), std::end(long_strings));

  for (auto _ : state) {
    for (const auto &key : words)
    {
      total += key.size();
    }
    benchmark::DoNotOptimize(total);
  }
}
BENCHMARK(Alpha2);
*/

static void Monotonic_Vector_Accidentally_Reuse_Alloc(benchmark::State &state)
{
  //  int total = 0;
  //  std::array<std::uint8_t, 10000> buffer{};
  //  std::pmr::monotonic_buffer_resource mem_resource{buffer.long_strings(), buffer.size()};
  //std::pmr::unsynchronized_pool_resource mem_resource;
  //std::pmr::monotonic_buffer_resource mem_resource;
  //

  std::pmr::monotonic_buffer_resource mem_resource(sizeof(std::pmr::string) * 1000);
  std::pmr::polymorphic_allocator alloc{ &mem_resource };

  for (auto _ : state) {
    [[maybe_unused]] std::pmr::vector<std::pmr::string> words{ std::begin(long_strings), std::end(long_strings), &mem_resource };
    benchmark::DoNotOptimize(words);
  }
}
BENCHMARK(Monotonic_Vector_Accidentally_Reuse_Alloc);

static void Monotonic_Vector_Heap_Copy_Destroy(benchmark::State &state)
{
  // Remember that free with monotonic buffer is a noop, so destruction is much faster
  std::pmr::vector<std::pmr::string> orig{ std::begin(long_strings), std::end(long_strings) };

  for (auto _ : state) {
    std::pmr::monotonic_buffer_resource mem_resource(sizeof(std::pmr::string) * 1000);
    std::pmr::polymorphic_allocator alloc{ &mem_resource };

    // forget the mem_resource here and everything goes to crap for performance
    [[maybe_unused]] std::pmr::vector<std::pmr::string> words{ orig, &mem_resource };
    benchmark::DoNotOptimize(words);
  }
}
BENCHMARK(Monotonic_Vector_Heap_Copy_Destroy);


static void Monotonic_Set_Heap_Sequential_Access(benchmark::State &state)
{
  std::pmr::monotonic_buffer_resource mem_resource(sizeof(std::pmr::string) * 1000);
  std::pmr::polymorphic_allocator alloc{ &mem_resource };

  [[maybe_unused]] const auto words = std::pmr::set<std::pmr::string>{ std::begin(long_strings), std::end(long_strings), &mem_resource };
  for (auto _ : state) {
    std::size_t total_len{ 0 };
    for (const auto &str : words) {
      total_len += str.size();
    }
    benchmark::DoNotOptimize(total_len);
  }
}
BENCHMARK(Monotonic_Set_Heap_Sequential_Access);

static void Std_Set_Sequential_Access(benchmark::State &state)
{
  const std::set<std::string> words{ std::begin(long_strings), std::end(long_strings) };
  for (auto _ : state) {
    std::size_t total_len{ 0 };
    for (const auto &str : words) {
      total_len += str.size();
    }
    benchmark::DoNotOptimize(total_len);
  }
}
BENCHMARK(Std_Set_Sequential_Access);

static void Monotonic_Set_StringView_Heap_Non_Sequential_Access(benchmark::State &state)
{
  std::pmr::monotonic_buffer_resource mem_resource(sizeof(std::pmr::string) * 1000);
  std::pmr::polymorphic_allocator alloc{ &mem_resource };

  const auto words = std::pmr::set<std::string_view>{ std::begin(long_string_views), std::end(long_string_views), &mem_resource };
  for (auto _ : state) {
    std::size_t len{ 0 };
    for (const auto &value : long_string_views) {
      len += words.find(value)->size();
    }
    benchmark::DoNotOptimize(len);
  }
}
BENCHMARK(Monotonic_Set_StringView_Heap_Non_Sequential_Access);

static void Std_Set_StringView_Non_Sequential_Access(benchmark::State &state)
{
  const std::set<std::string_view> words{ std::begin(long_string_views), std::end(long_string_views) };
  for (auto _ : state) {
    std::size_t len{ 0 };
    for (const auto &value : long_string_views) {
      len += words.find(value)->size();
    }
    benchmark::DoNotOptimize(len);
  }
}
BENCHMARK(Std_Set_StringView_Non_Sequential_Access);


struct LongStringViews
{
  auto begin() const
  {
    return long_string_views.begin();
  }
  auto end() const
  {
    return long_string_views.end();
  }
};

struct LongCharStrings
{
  auto begin() const
  {
    return std::begin(long_strings);
  }
  auto end() const
  {
    return std::end(long_strings);
  }
};

template<std::size_t Size>
struct Monotonic
{
  std::pmr::monotonic_buffer_resource mem_resource{ Size };
  std::pmr::polymorphic_allocator<> alloc{ &mem_resource };

  template<typename Container>
  auto create(const auto &dataSource)
  {
    if constexpr (requires { Container{}.bucket_count(); }) {
      return Container{ dataSource.begin(), dataSource.end(), Container{}.bucket_count(), alloc };
    } else {
      return Container{ dataSource.begin(), dataSource.end(), alloc };
    }
  }
};

template<std::size_t Size>
struct Monotonic_Wink_Out
{
  std::pmr::monotonic_buffer_resource mem_resource{ Size };
  std::pmr::polymorphic_allocator<> alloc{ &mem_resource };

  template<typename Container>
  auto create(const auto &dataSource)
  {
    if constexpr (requires { Container{}.bucket_count(); }) {
      return alloc.new_object<Container>(dataSource.begin(), dataSource.end(), Container{}.bucket_count());
    } else {
      return alloc.new_object<Container>(dataSource.begin(), dataSource.end());
    }
  }
};


struct Stack
{
  template<typename Container>
  auto create(const auto &dataSource)
  {
    return Container{ dataSource.begin(), dataSource.end() };
  }
};

struct Noop
{
  constexpr auto do_op(const auto &) const noexcept {}
};

struct Sort
{
  auto do_op(auto *container)
  {
    return do_op(*container);
  }
  auto do_op(auto &container)
  {
    std::sort(container.begin(), container.end());
  }
};

struct InsertAtFront
{
  auto do_op(auto *container)
  {
    return do_op(*container);
  }
  auto do_op(auto &container)
  {
    container.insert(container.begin(), container.back());
  }
};

struct InsertDeleteAtFront
{
  auto do_op(auto *container)
  {
    return do_op(*container);
  }
  auto do_op(auto &container)
  {
    container.insert(container.begin(), container.back());
    container.erase(container.begin());
  }
};

struct AddMiddleChar
{
  auto do_op(auto *container)
  {
    return do_op(*container);
  }

  auto do_op(auto &container)
  {
    char result = 0;
    for (const auto &value : container) {
      result += value[value.size() / 2];
    }
    for (auto itr = container.rbegin();
         itr != container.rend();
         ++itr) {
      result += (*itr)[0];
    }
    for (const auto &value : container) {
      result += value.back();
    }
    benchmark::DoNotOptimize(result);
    return result;
  }
};

template<typename Container, typename DataSource, typename Allocator, typename Operation = Noop>
static void CreateFree(benchmark::State &state)
{
  DataSource ds;
  Operation op;
  for (auto _ : state) {
    Allocator alloc;
    auto values = alloc.template create<Container>(ds);
    op.do_op(values);
    benchmark::DoNotOptimize(values);
  }
}

template<typename Container, typename DataSource, typename Allocator, typename Operation = Noop>
static void Op(benchmark::State &state)
{
  DataSource ds;
  Operation op;
  Allocator alloc;
  auto values = alloc.template create<Container>(ds);

  for (auto _ : state) {
    op.do_op(values);
  }
  benchmark::DoNotOptimize(values);
}


BENCHMARK_TEMPLATE(CreateFree, std::set<std::string>, LongCharStrings, Stack);
BENCHMARK_TEMPLATE(CreateFree, std::set<std::string_view>, LongCharStrings, Stack);
BENCHMARK_TEMPLATE(CreateFree, std::set<std::string_view>, LongStringViews, Stack);
BENCHMARK_TEMPLATE(CreateFree, std::unordered_set<std::string>, LongCharStrings, Stack);
BENCHMARK_TEMPLATE(CreateFree, std::unordered_set<std::string_view>, LongCharStrings, Stack);
BENCHMARK_TEMPLATE(CreateFree, std::unordered_set<std::string_view>, LongStringViews, Stack);

BENCHMARK_TEMPLATE(CreateFree, std::pmr::set<std::pmr::string>, LongCharStrings, Monotonic<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::set<std::pmr::string>, LongCharStrings, Monotonic_Wink_Out<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::set<std::string_view>, LongCharStrings, Monotonic<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::set<std::string_view>, LongStringViews, Monotonic<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::unordered_set<std::pmr::string>, LongCharStrings, Monotonic<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::unordered_set<std::string_view>, LongCharStrings, Monotonic<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::unordered_set<std::string_view>, LongStringViews, Monotonic<16384>);

BENCHMARK_TEMPLATE(CreateFree, std::pmr::unordered_set<std::pmr::string>, LongCharStrings, Monotonic_Wink_Out<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::unordered_set<std::string_view>, LongStringViews, Monotonic_Wink_Out<16384>);

BENCHMARK_TEMPLATE(CreateFree, std::vector<std::string>, LongCharStrings, Stack);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::vector<std::pmr::string>, LongCharStrings, Monotonic<16384>);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::vector<std::pmr::string>, LongCharStrings, Monotonic_Wink_Out<16384>);

BENCHMARK_TEMPLATE(CreateFree, std::vector<std::string>, LongCharStrings, Stack, Sort);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::vector<std::pmr::string>, LongCharStrings, Monotonic<16384>, Sort);
BENCHMARK_TEMPLATE(CreateFree, std::pmr::vector<std::pmr::string>, LongCharStrings, Monotonic_Wink_Out<16384>, Sort);

BENCHMARK_TEMPLATE(Op, std::vector<std::string>, LongCharStrings, Stack, InsertDeleteAtFront);
BENCHMARK_TEMPLATE(Op, std::pmr::vector<std::pmr::string>, LongCharStrings, Monotonic<1638400>, InsertDeleteAtFront);
BENCHMARK_TEMPLATE(Op, std::list<std::string>, LongCharStrings, Stack, InsertDeleteAtFront);
BENCHMARK_TEMPLATE(Op, std::pmr::list<std::pmr::string>, LongCharStrings, Monotonic<1638400>, InsertDeleteAtFront);

BENCHMARK_TEMPLATE(Op, std::vector<std::string>, LongCharStrings, Stack, AddMiddleChar);
BENCHMARK_TEMPLATE(Op, std::pmr::vector<std::pmr::string>, LongCharStrings, Monotonic<16384>, AddMiddleChar);
BENCHMARK_TEMPLATE(Op, std::list<std::string>, LongCharStrings, Stack, AddMiddleChar);
BENCHMARK_TEMPLATE(Op, std::pmr::list<std::pmr::string>, LongCharStrings, Monotonic<16384>, AddMiddleChar);
BENCHMARK_TEMPLATE(Op, std::set<std::string>, LongCharStrings, Stack, AddMiddleChar);
BENCHMARK_TEMPLATE(Op, std::pmr::set<std::pmr::string>, LongCharStrings, Monotonic<16384>, AddMiddleChar);




static void Std_Set_Heap_Create_Leak(benchmark::State &state)
{
  for (auto _ : state) {
    auto *words = new std::set<std::string>{ std::begin(long_strings), std::end(long_strings) };
    benchmark::DoNotOptimize(*words);
  }
}
BENCHMARK(Std_Set_Heap_Create_Leak);


static void Std_Vector_Copy_Destroy(benchmark::State &state)
{
  const auto orig = std::vector<std::string>(std::begin(long_strings), std::end(long_strings));
  for (auto _ : state) {
    [[maybe_unused]] auto words = orig;
    benchmark::DoNotOptimize(words);
  }
}
BENCHMARK(Std_Vector_Copy_Destroy);
