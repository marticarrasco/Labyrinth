#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <algorithm>

/*Constants*/
sf::Vector2f windowSize = sf::Vector2f(1600, 900);
sf::Vector2f tileSize = sf::Vector2f(20, 20);
sf::Vector2i mapSize = sf::Vector2i(70, 40);
sf::Vector2f mapSizePixel = sf::Vector2f(tileSize.x * mapSize.x, tileSize.y * mapSize.y);
sf::Vector2f mapPosition = (windowSize - mapSizePixel) / 2.0f;
sf::Color mapColor = sf::Color(180, 180, 180);

float wallThickness = 4;
sf::Color wallColor = sf::Color(80, 80, 80);

float userRadius = 7;
sf::Color userColor = sf::Color::Red;

const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;

int opposite(int dir) {
	return (dir + 2) % 4;
}

const sf::Vector2i directions[] = { sf::Vector2i(0, -1), sf::Vector2i(1, 0), sf::Vector2i(0, 1), sf::Vector2i(-1, 0) };

class Tile {
	sf::RectangleShape m_northWall, m_southWall, m_eastWall, m_westWall;
	sf::Vector2f m_pixelPos;
	sf::Vector2i m_coords;
	sf::Vector2i m_representative;

	sf::RectangleShape m_highlight;
	bool m_isHighlighted = false;

	std::vector<bool> m_adjacency = std::vector<bool>(4, true);

	sf::CircleShape m_character;
	bool m_hasCharacter = false;

	bool m_visited = false;

public:
	void setPosition(sf::Vector2i coords) {
		m_coords = coords;
		m_representative = coords;

		m_pixelPos = sf::Vector2f(mapPosition + sf::Vector2f(m_coords.x * tileSize.x, m_coords.y * tileSize.y));

		if (m_coords.x == 0) m_adjacency[WEST] = false;
		if (m_coords.y == 0) m_adjacency[NORTH] = false;
		if (m_coords.x == mapSize.x - 1) m_adjacency[EAST] = false;
		if (m_coords.y == mapSize.y - 1) m_adjacency[SOUTH] = false;


		m_northWall.setPosition(m_pixelPos + sf::Vector2f(0, -wallThickness/2));
		m_southWall.setPosition(m_pixelPos + sf::Vector2f(0, tileSize.y - wallThickness / 2));
		m_westWall.setPosition(m_pixelPos + sf::Vector2f(-wallThickness / 2, 0));
		m_eastWall.setPosition(m_pixelPos + sf::Vector2f(tileSize.x - wallThickness / 2, 0));
	
		m_highlight.setPosition(m_pixelPos);

		m_character.setPosition(m_pixelPos + sf::Vector2f(tileSize.x/2 - userRadius, tileSize.y/2 - userRadius));
	}

	sf::Vector2i getRepresentative() {
		return m_representative;
	}

	void setRep(sf::Vector2i rep) {
		m_representative = rep;
	}

	void setCharacter(bool b) {
		m_hasCharacter = b;
	}

	Tile() {
		m_northWall.setSize(sf::Vector2f(tileSize.x, wallThickness));
		m_northWall.setFillColor(wallColor);

		m_southWall.setSize(sf::Vector2f(tileSize.x, wallThickness));
		m_southWall.setFillColor(wallColor);

		m_eastWall.setSize(sf::Vector2f(wallThickness, tileSize.y));
		m_eastWall.setFillColor(wallColor);

		m_westWall.setSize(sf::Vector2f(wallThickness, tileSize.y));
		m_westWall.setFillColor(wallColor);

		m_character.setRadius(userRadius);
		m_character.setFillColor(userColor);

		m_highlight.setSize(tileSize);
		m_highlight.setFillColor(sf::Color(255, 255, 0));
	}

	void draw(sf::RenderWindow& window) {
		if (m_isHighlighted) window.draw(m_highlight);
		if (!m_adjacency[NORTH]) window.draw(m_northWall);
		if (!m_adjacency[SOUTH]) window.draw(m_southWall);
		if (!m_adjacency[EAST]) window.draw(m_eastWall);
		if (!m_adjacency[WEST]) window.draw(m_westWall);
		if (m_hasCharacter) window.draw(m_character);
	}

	void removeAdj(int dir) {
		m_adjacency[dir] = false;
	}

	bool isConnected(int dir) {
		return m_adjacency[dir];
	}

	void setHighlight(bool b) {
		m_isHighlighted = b;
	}

	void setVisited(bool b) {
		m_visited = b;
	}

	bool isVisited() {
		return m_visited;
	}
};

typedef std::vector<Tile> VT;
typedef std::vector<VT> VVT;

typedef std::vector<int> VI;
typedef std::vector<VI> VVI;
typedef std::vector<VVI> VVVI;

typedef std::pair<sf::Vector2i, int> PII;



class Level {
	VVT m_tileMap;
	sf::RectangleShape m_background;
	sf::Vector2i m_characterPos = sf::Vector2i(0,0);


public:
	void drawAll(sf::RenderWindow& window) {
		window.draw(m_background);
		for (int i = 0; i < mapSize.x; ++i) {
			for (int j = 0; j < mapSize.y; ++j) {
				m_tileMap[i][j].draw(window);
			}
		}
	}

	void setCharacterPosition(sf::Vector2i pos) {
		m_tileMap[m_characterPos.x][m_characterPos.y].setCharacter(false);
		m_tileMap[pos.x][pos.y].setCharacter(true);
		m_characterPos = pos;
	}

	Level() :
		m_tileMap(mapSize.x, VT(mapSize.y))
	{
		m_background.setSize(mapSizePixel);
		m_background.setFillColor(mapColor);
		m_background.setPosition(mapPosition);

		for (int i = 0; i < mapSize.x; ++i) {
			for (int j = 0; j < mapSize.y; ++j) {
				m_tileMap[i][j].setPosition(sf::Vector2i(i,j));
			}
		}

		kruskal();
		setCharacterPosition(sf::Vector2i(0, 0));
	}

	sf::Vector2i getRep(sf::Vector2i pos) {
		sf::Vector2i rep = m_tileMap[pos.x][pos.y].getRepresentative();
		if (rep == pos) return rep;
		rep = getRep(rep);
		m_tileMap[pos.x][pos.y].setRep(rep);
		return rep;
	}

	void kruskal() {
		std::vector<PII> adj = std::vector<PII>(0);
		for (int i = 0; i < mapSize.x; ++i) {
			for (int j = 0; j < mapSize.y; ++j) {
				if (j != 0) adj.push_back(PII(sf::Vector2i(i, j), NORTH));
				if (i != 0) adj.push_back(PII(sf::Vector2i(i, j), WEST));
			}
		}

		std::random_shuffle(adj.begin(), adj.end());

		for (size_t i = 0; i < adj.size(); ++i) {
			sf::Vector2i tile1 = adj[i].first;
			sf::Vector2i tile2 = adj[i].first + directions[adj[i].second];
			sf::Vector2i r1 = getRep(tile1);
			sf::Vector2i r2 = getRep(tile2);
			
			if (r1 == r2) {
				m_tileMap[tile1.x][tile1.y].removeAdj(adj[i].second);
				m_tileMap[tile2.x][tile2.y].removeAdj(opposite(adj[i].second));
			}
			else {
				m_tileMap[r1.x][r1.y].setRep(tile2);
			}
		}
	}

	void moveCharacter(int dir) {
		bool canMove = m_tileMap[m_characterPos.x][m_characterPos.y].isConnected(dir);
		if (!canMove) return;
		sf::Vector2i newPos = m_characterPos + directions[dir];
		setCharacterPosition(m_characterPos + directions[dir]);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
			resetHighlight();
			highlightSolution();
		}
	}

	bool highlight(sf::Vector2i pos) {
		bool ans = false;
		if (pos.x == mapSize.x - 1 && pos.y == mapSize.y - 1) {
			m_tileMap[pos.x][pos.y].setHighlight(true);
			return true;
		}
		m_tileMap[pos.x][pos.y].setVisited(true);
		for (int i = 0; i < 4; ++i) {
			if (!m_tileMap[pos.x][pos.y].isConnected(i)) continue;
			sf::Vector2i newPos = pos + directions[i];
			if (m_tileMap[newPos.x][newPos.y].isVisited()) continue;
			ans |= highlight(newPos);
		}
		if (ans) {
			//std::cout << "Found solution! :)" << std::endl;
			m_tileMap[pos.x][pos.y].setHighlight(true);
		}
		return ans;
	}

	void highlightSolution() {
		for (int i = 0; i < mapSize.x; ++i) {
			for (int j = 0; j < mapSize.y; ++j) {
				m_tileMap[i][j].setVisited(false);
			}
		}
		highlight(m_characterPos);
	}

	void resetHighlight() {
		for (int i = 0; i < mapSize.x; ++i) {
			for (int j = 0; j < mapSize.y; ++j) {
				m_tileMap[i][j].setHighlight(false);
			}
		}
	}

	void processEvent(sf::Event& e) {
		if (e.type == sf::Event::KeyPressed) {
			if (e.key.code == sf::Keyboard::Up) {
				moveCharacter(NORTH);
			}
			else if (e.key.code == sf::Keyboard::Down) {
				moveCharacter(SOUTH);
			}
			else if (e.key.code == sf::Keyboard::Left) {
				moveCharacter(WEST);
			}
			else if (e.key.code == sf::Keyboard::Right) {
				moveCharacter(EAST);
			}
			else if (e.key.code == sf::Keyboard::LShift) {
				highlightSolution();
			}
		}
		if (e.type == sf::Event::KeyReleased) {
			if (e.key.code == sf::Keyboard::LShift) {
				resetHighlight();
			}
		}
	}
};

class Game {
private:
	sf::RenderWindow m_window;
	sf::Event m_event;
	Level m_level;
public:
	void processEvents() {
		while (m_window.pollEvent(m_event)) {
			m_level.processEvent(m_event);
		}
	}

	void drawAll() {
		m_level.drawAll(m_window);
		//TODO
	}

	void update() {
		//TODO
	}

	void run() {
		while (m_window.isOpen()) {
			processEvents();
			m_window.clear();
			update();
			drawAll();
			m_window.display();
		}
	}

	Game() :
		m_window(sf::VideoMode(windowSize.x, windowSize.y), "Labyrinth")
	{
		
	}

};

int main()
{
	srand(time(NULL));
	Game game;
	game.run();
}
