#include <iostream>
#include<SFML/Graphics.hpp>
#include<SFPhysics.h>

using namespace std;
using namespace sf;
using namespace sfp;

void loadTex(Texture& tex, string filename) {
	if (!tex.loadFromFile(filename)) {
		cout << "Could not load " << filename << "!" << endl;
		exit(1);
	} //detect missing texture file
	else tex.loadFromFile(filename);
}

void MoveCrossbow(PhysicsSprite& crossbow, int eTimeMS) {
	if (crossbow.getCenter().x < 800 - crossbow.getSize().x / 2) { //prevent crossbow get out of range
		if (Keyboard::isKeyPressed(Keyboard::Right)) {
			Vector2f newPos(crossbow.getCenter());
			newPos.x = newPos.x + (eTimeMS);
			crossbow.setCenter(newPos);
		}
	}
	if (crossbow.getCenter().x > crossbow.getSize().x / 2) { //prevent crossbow get out of range
		if (Keyboard::isKeyPressed(Keyboard::Left)) {
			Vector2f newPos(crossbow.getCenter());
			newPos.x = newPos.x - (eTimeMS);
			crossbow.setCenter(newPos);
		}
	}
}

int main() {
	RenderWindow window(VideoMode(800, 600), "DuckGame");
	World world(Vector2f(0,0));
	bool restart = true;

	while (restart == true) {
		restart = false;
		int score = 0;
		int arrowRemain = 5;

		//add crossbow
		PhysicsSprite crossbow;
		Texture crossbowTex;

		loadTex(crossbowTex, "images/crossbow.png");
		crossbow.setTexture(crossbowTex);
		Vector2f sz = crossbow.getSize();
		crossbow.setCenter(Vector2f(400, 600 - (sz.y / 2)));
		world.AddPhysicsBody(crossbow);

		//add arrows
		PhysicsSprite arrow;
		Texture arrowTex;
		loadTex(arrowTex, "images/arrow.png");
		arrow.setTexture(arrowTex);
		bool drawingArrow(false);

		//set top
		PhysicsRectangle top;
		top.setSize(Vector2f(1000, 10));
		top.setCenter(Vector2f(400, -10));
		top.setStatic(true);
		world.AddPhysicsBody(top);

		//set top collision
		top.onCollision = [&drawingArrow, &world, &arrow](PhysicsBodyCollisionResult result) {
			drawingArrow = false;
			world.RemovePhysicsBody(arrow);
		};

		//set walls
		PhysicsRectangle left;
		left.setSize(Vector2f(10, 1000));
		left.setStatic(true);
		world.AddPhysicsBody(left);
		PhysicsRectangle right;
		right.setSize(Vector2f(10, 1000));
		right.setStatic(true);
		world.AddPhysicsBody(right);

		//set ducks
		Texture duckTex;
		loadTex(duckTex, "images/duck1.png");
		PhysicsShapeList<PhysicsSprite> ducks;
		int duckSize;
		for (int i(0); i < 6; i++) {
			PhysicsSprite& duck = ducks.Create();
			duck.setTexture(duckTex);
			int x = 50 + ((700 / 5) * i);
			Vector2f sz = duck.getSize();
			duck.setCenter(Vector2f(x, 20 + (sz.y / 2)));
			duck.setVelocity(Vector2f(0.25, 0));
			world.AddPhysicsBody(duck);
			duckSize = duck.getSize().x;

			duck.onCollision = [&world, &arrow, &right, &duck, &ducks, &x, &sz, &score, &drawingArrow](PhysicsBodyCollisionResult result) {
				if (result.object2 == right) {
					world.RemovePhysicsBody(duck);
					//ducks.QueueRemove(duck);
					//duck.setCenter(Vector2f(x, 20 + (sz.y / 2)));
					//world.AddPhysicsBody(duck);
				}
				else if (result.object2 == arrow) {
					drawingArrow = false;
					world.RemovePhysicsBody(arrow);
					world.RemovePhysicsBody(duck);
					ducks.QueueRemove(duck);
					score += 10;
				}
			};
		}

		left.setCenter(Vector2f(-10 - duckSize, 300));
		right.setCenter(Vector2f(810 + duckSize, 300));

		//set clock
		Clock clock;
		Time lastTime(clock.getElapsedTime());

		//set text
		Text scoreText;
		Font font;
		if (!font.loadFromFile("font/arial.ttf")) {
			cout << "Couldn't load font arial.ttf" << endl;
			exit(1);
		}
		scoreText.setFont(font);
		Text arrowCountText;
		arrowCountText.setFont(font);

		do {

			//set Elapsed Time
			Time currentTime(clock.getElapsedTime());
			Time deltaTime = currentTime - lastTime;
			long deltaTimeMS(deltaTime.asMilliseconds());

			//control
			if (deltaTimeMS > 9) {
				world.UpdatePhysics(deltaTimeMS);
				lastTime = currentTime;

				MoveCrossbow(crossbow, deltaTimeMS);

				//shooting arrows
				if (Keyboard::isKeyPressed(Keyboard::Space) && !drawingArrow) {
					drawingArrow = true;
					arrow.setCenter(crossbow.getCenter());
					arrow.setVelocity(Vector2f(0, -1));
					world.AddPhysicsBody(arrow);
					arrowRemain -= 1;
				}

				window.clear();
				if (drawingArrow) {
					window.draw(arrow);
				}
				for (auto& duck : ducks) {
					window.draw((PhysicsSprite&)duck);
				}
				window.draw(top);
				window.draw(left);
				window.draw(right);
				window.draw(crossbow);

				scoreText.setString(to_string(score));
				FloatRect textBounds = scoreText.getGlobalBounds();
				scoreText.setPosition(
					Vector2f(790 - textBounds.width, 590 - textBounds.height));
				window.draw(scoreText);
				arrowCountText.setString(to_string(arrowRemain));
				textBounds = arrowCountText.getGlobalBounds();
				arrowCountText.setPosition(
					Vector2f(10, 590 - textBounds.height));
				window.draw(arrowCountText);


			}

			window.display();
			ducks.DoRemovals();
		} while (arrowRemain > 0 || drawingArrow);

		Text gameOverText;
		gameOverText.setFont(font);
		if (score < 50) {
			gameOverText.setString("GAME OVER! \n YOUR FINAL SCORE IS: " + to_string(score));
		}
		else {
			gameOverText.setString("YOU WIN! \n YOUR FINAL SCORE IS: " + to_string(score));
		}


		FloatRect textBounds = gameOverText.getGlobalBounds();
		gameOverText.setPosition(Vector2f(400 - (textBounds.width / 2), 300 - (textBounds.height / 2)));
		window.draw(gameOverText);
		window.display();
		while (restart == false) {
			if (Keyboard::isKeyPressed(Keyboard::Space)) {
				window.clear();
				restart = true;
			}
		}
	}
}