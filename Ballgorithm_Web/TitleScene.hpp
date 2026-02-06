#pragma once
# include <Siv3D.hpp>

class Game;

class TitleScene {
public:
	void update(Game& game);
	void draw() const;

private:
	double m_time = 0.0;
};
