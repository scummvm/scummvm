// ...existing code...

#include <string>
#include <vector>

// Assuming Game is a class or struct defined elsewhere
class Game {
	// Placeholder definition for Game
public:
	std::string id;
};

class GameListWidget {
	// ...existing code...

private:
	std::string _currentFilter;          // Stores the current filter
	std::vector<Game> _filteredGameList; // Stores the filtered list of games

	void applyFilter(const std::string &filter);
	int findFilteredGameIndex(const std::string &gameId);
	void setSelectedGame(const Game &game);
	void clearSelection();
	void refreshGameList();

	// ...existing code...
};
