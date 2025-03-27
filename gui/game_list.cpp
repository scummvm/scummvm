#include <algorithm>
#include <cctype> // For tolower()
#include <string>
#include <vector>

using String = std::string;

class GameListWidget {
public:
	void removeGame(const String &gameId);
	void applyFilter(const String &filter);

private:
	std::vector<String> _gameList;
	std::vector<String> _filteredGameList;
	String _currentFilter;
	String _selectedGameId; // To track currently selected game

	int findFilteredGameIndex(const String &gameId) const;
	bool gameMatchesFilter(const String &game, const String &filter) const;
	void setSelectedGame(const String &gameId);
	void clearSelection();
	void refreshGameList();
};

// Finds the index of a game in the filtered list, or -1 if not found
int GameListWidget::findFilteredGameIndex(const String &gameId) const {
	auto it = Common::find(_filteredGameList.begin(), _filteredGameList.end(), gameId);
	if (it != _filteredGameList.end()) {
		return static_cast<int>(std::distance(_filteredGameList.begin(), it));
	}
	return -1;
}

// Case-insensitive check if game matches filter
bool GameListWidget::gameMatchesFilter(const String &game, const String &filter) const {
	if (filter.empty())
		return true;

	auto containsCaseInsensitive = [](const String &str, const String &sub) {
		auto it = std::search(
			str.begin(), str.end(),
			sub.begin(), sub.end(),
			[](char ch1, char ch2) {
				return std::tolower(ch1) == std::tolower(ch2);
			});
		return it != str.end();
	};

	return containsCaseInsensitive(game, filter);
}

// Sets the currently selected game
void GameListWidget::setSelectedGame(const String &gameId) {
	_selectedGameId = gameId;
	// In a real implementation, you would also update the UI here
}

// Clears the current selection
void GameListWidget::clearSelection() {
	_selectedGameId.clear();
	// In a real implementation, you would also update the UI here
}

// Refreshes the game list display
void GameListWidget::refreshGameList() {
	// In a real implementation, this would update the UI widget
	// to reflect the current state of _filteredGameList
	// This might involve:
	// 1. Clearing the current display
	// 2. Repopulating with items from _filteredGameList
	// 3. Restoring selection if needed
}

// Existing implementations...

void GameListWidget::removeGame(const String &gameId) {
	int removedIndex = findFilteredGameIndex(gameId);
	_gameList.erase(std::remove(_gameList.begin(), _gameList.end(), gameId), _gameList.end());
	applyFilter(_currentFilter);

	if (!_filteredGameList.empty()) {
		int newIndex = std::min(removedIndex, static_cast<int>(_filteredGameList.size()) - 1);
		setSelectedGame(_filteredGameList[newIndex]);
	} else {
		clearSelection();
	}
	refreshGameList();
}

void GameListWidget::applyFilter(const String &filter) {
	_currentFilter = filter;
	_filteredGameList.clear();

	for (const auto &game : _gameList) {
		if (gameMatchesFilter(game, filter)) {
			_filteredGameList.push_back(game);
		}
	}
}