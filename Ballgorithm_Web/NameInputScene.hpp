#pragma once

# include <Siv3D.hpp>
# include "TextBox.h"

class Game;

class NameInputScene {
public:
	NameInputScene();
	void update(Game& game);
	void draw() const;

private:
	mutable TextBox m_textBox;
	double m_time = 0.0;
	bool m_showError = false;
	String m_prevText;
};
