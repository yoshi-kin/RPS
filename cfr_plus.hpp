#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <bitset>
#include <fstream>
#include <string>
#include <memory>
#include <iterator>

class CFR;
class Game {
public:
  Game(): actions({"R", "P", "S"}), players({0, 1}), history("") {}
  Game(Game& game) {
    this->players = game.players;
    this->history = game.history;
		this->actions = game.actions;
  }

	// @brief proceed game
	// @param action action that the player takes on the node
  std::string step(std::string action) {
    history += action;
    std::rotate(players.begin(), players.begin() + players.size() - 1, players.end());
    return history;
  }
	
	// @return index of current player
  int& turn() { return players[0]; }
	
	// @return whether the node is terminal 
  int terminal() {
    if (history.size() == 2) return true;
		return false;
  }

	// @return utility for leaf node
  int evaluate() {
    if (history == "RR") return 0.;
    if (history == "RP") return -1.;
    if (history == "RS") return 2.;
    if (history == "PR") return 1.;
    if (history == "PP") return 0.;
    if (history == "PS") return -2.;
    if (history == "SR") return -2.;
    if (history == "SP") return 2.;
    if (history == "SS") return 0.;
		return 0;
  }

private:
  std::vector<std::string> actions;
  std::vector<int> players;
  std::string history;
	friend CFR;
};

class CFR {
public:
	// @brief init strategy and regret in each information set
	// @param game game
	CFR(Game& game) {
		std::map<std::string, double> strategy;
		std::map<std::string, double> regret;
		sumStrategyArray = {{{"R", 0.}, {"P", 0.}, {"S", 0.}}, {{"R", 0.}, {"P", 0.}, {"S", 0.}}};
		for (auto action : game.actions) {
			strategy[action] = 1. / game.actions.size();
			regret[action] = 0. ;
		}
		for (int i = 0; i < game.players.size(); ++i) {
			strategyArray.push_back(strategy);
			regretArray.push_back(regret);
		}
	}

	// @brief minimize counter factual regret
	// @param game next game state
	// @param p1 probability of player_1 reaching the node 
	//           when player_1 plays according to strategy
	// @param p2 probability of player_2 reaching the node 
	//           when player_2 plays according to strategy
  double expect_utility_recursively(Game& game, double p1, double p2){
    if (game.terminal()) {
			return game.evaluate();
		}
		double expected_utility = 0.;
		std::map<std::string, double> utility;
    for (auto action : game.actions) {
    	Game game_cp(game);
      game_cp.step(action);
			utility[action] = game.turn() == 0
				? expect_utility_recursively(game_cp, p1 * strategyArray[game.turn()][action], p2) * p2 * -1.
				: expect_utility_recursively(game_cp, p1, p2 * strategyArray[game.turn()][action]) * p1 * -1. ;
			expected_utility += utility[action] * strategyArray[game.turn()][action];
    }
		for (auto& [action, regret] : regretArray[game.turn()]) {
			regretArray[game.turn()][action] += (utility[action] - expected_utility);
			// reset negative over all regret
			if (regretArray[game.turn()][action] < 0) regretArray[game.turn()][action] = 0;
		}
    return expected_utility;
  }

	// @brief update strategy in each information set for both players
	void update_strategy() {
		for (int i = 0; i < strategyArray.size(); ++i) {
			double sum_regret = 0;
			for (auto& [action, regret] : regretArray[i]) {
				sum_regret += std::max(regretArray[i][action], 0. );
			}
			for (auto& [action, probability] : strategyArray[i]) {
				strategyArray[i][action] = sum_regret > 0
					? std::max(regretArray[i][action], 0.) / sum_regret
					: 1. / strategyArray[i].size();
			}
		}
	}

	// @brief update sum_strategy in each information set for both players
	void update_sum_strategy() {
		for (int i = 0; i < sumStrategyArray.size(); ++i) {
			for (auto& [action, probability] : sumStrategyArray[i]) {
				sumStrategyArray[i][action] += strategyArray[i][action];
			}
		}
	}

	// @brief update strategy in specified information set
	void update_single_strategy(int infoset) {
		double sum_regret = 0;
		for (auto& [action, regret] : regretArray[infoset]) {
			sum_regret += std::max(regretArray[infoset][action], 0. );
		}
		for (auto& [action, probability] : strategyArray[infoset]) {
			strategyArray[infoset][action] = sum_regret > 0
				? std::max(regretArray[infoset][action], 0.) / sum_regret
				: 1. / strategyArray[infoset].size();
		}
	}

	// @brief minimize counterfactual regret and update strategy
	void minimize(Game& game) {
		double expected_utility = expect_utility_recursively(game, 1., 1.);
		update_strategy();
		update_sum_strategy();
		
		// std::cout << utility << std::endl;
	}

	// @return strategy in each information set
	std::vector<std ::map<std::string, double>>& getStrategyArray() { return strategyArray; }
	// @return strategy in each information set
	std::vector<std ::map<std::string, double>>& getSumStrategyArray() { return sumStrategyArray; }
	// @return counterfactual regret in each information set
	std::vector<std::map<std::string, double>>& getRegretArray() { return regretArray; }

private:
  std::vector<std::map<std::string, double>> strategyArray;
	std::vector<std::map<std::string, double>> sumStrategyArray;
  std::vector<std::map<std::string, double>> regretArray;
};
