#include <SFML/Graphics.hpp>
#include <iostream>
#include <functional>
#include "imgui-SFML.h"
#include "imgui.h"

float createMatrix(const std::vector<std::vector<float>> &matrix)
{
	return matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
		matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
		matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
}

sf::Color interpolateColors(const sf::Color &color1, const sf::Color &color2, float t)
{
	float r = color1.r + (color2.r - color1.r) * t;
	float g = color1.g + (color2.g - color1.g) * t;
	float b = color1.b + (color2.b - color1.b) * t;
	float a = color1.a + (color2.a - color1.a) * t;

	return sf::Color(static_cast<sf::Uint8>(r), static_cast<sf::Uint8>(g), static_cast<sf::Uint8>(b),
					 static_cast<sf::Uint8>(a));
}

class RClass : public sf::Sprite
{
public:
	void Create(const sf::Vector2u &size, const int selectedNormalIndex)
	{
		image.create(size.x, size.y, sf::Color::Cyan);
		texture.loadFromImage(image);
		setTexture(texture);

		fColor = sf::Color::Black;
		sColor = sf::Color::White;

		index = selectedNormalIndex;
	}

	void DrawRFunc(const std::function<float(const sf::Vector2f &)> &rfunc, const sf::FloatRect &subSpace)
	{
		sf::Vector2f spaceStep = {subSpace.width / static_cast<float>(image.getSize().x),
								  subSpace.height / static_cast<float>(image.getSize().y)};

		for (int x = 0; x < image.getSize().x - 1; ++x)
		{
			for (int y = 0; y < image.getSize().y - 1; ++y)
			{
				sf::Vector2f spacePointFirst = {subSpace.left + static_cast<float>(x) * spaceStep.x,
												subSpace.top + static_cast<float>(y) * spaceStep.y};

				const float z1 = rfunc(spacePointFirst);

				sf::Vector2f spacePointSecond = {subSpace.left + static_cast<float>(x + 1) * spaceStep.x,
												 subSpace.top + static_cast<float>(y) * spaceStep.y};

				const float z2 = rfunc(spacePointSecond);

				sf::Vector2f spacePointThird = {subSpace.left + static_cast<float>(x) * spaceStep.x,
												subSpace.top + static_cast<float>(y + 1) * spaceStep.y};

				const float z3 = rfunc(spacePointThird);

				const float A = createMatrix({
					{spacePointFirst.y, z1, 1},
					{spacePointSecond.y, z2, 1},
					{spacePointThird.y, z3, 1},
				});

				const float B = createMatrix({
					{spacePointFirst.x, z1, 1},
					{spacePointSecond.x, z2, 1},
					{spacePointThird.x, z3, 1},
				});

				const float C = createMatrix({
					{spacePointFirst.x, spacePointFirst.y, 1},
					{spacePointSecond.x, spacePointSecond.y, 1},
					{spacePointThird.x, spacePointThird.y, 1},
				});

				const float D = createMatrix({
					{spacePointFirst.x, spacePointFirst.y, z1},
					{spacePointSecond.x, spacePointSecond.y, z2},
					{spacePointThird.x, spacePointThird.y, z3},
				});

				const float rat = std::sqrt(A * A + B * B + C * C + D * D);

				float nx = A / rat, ny = B / rat, nz = C / rat, nw = D / rat;

				float selectedNormal = nx;

				switch (index)
				{
				case 0:
					break;
				case 1:
					selectedNormal = ny;
					break;
				case 2:
					selectedNormal = nz;
					break;
				case 3:
					selectedNormal = nw;
					break;
				}

				auto pixelColor = interpolateColors(fColor, sColor, (1.f + selectedNormal) / 2);
				image.setPixel(x, y, pixelColor);
			}
		}

		texture.update(image);
	}

	void UpdatePalette(const sf::Color &firstColor, const sf::Color &secondColor)
	{
		for (int x = 0; x < image.getSize().x - 1; ++x)
		{
			for (int y = 0; y < image.getSize().y - 1; ++y)
			{
				float t = (static_cast<float>(image.getPixel(x, y).r) - fColor.r) / (sColor.r - fColor.r);
				auto pixelColor = interpolateColors(firstColor, secondColor, t);
				image.setPixel(x, y, pixelColor);
			}
		}

		fColor = firstColor;
		sColor = secondColor;
		texture.update(image);
	}

	void SaveImageToFile(const std::string &filename) { image.saveToFile(filename); }

private:
	sf::Color fColor;
	sf::Color sColor;
	sf::Texture texture;
	sf::Image image;
	int index;
};

float RAnd(float w1, float w2) { return w1 + w2 + std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }

float ROr(float w1, float w2) { return w1 + w2 - std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }

std::vector<RClass *> sprites;
int main()
{
	sf::RenderWindow window(sf::VideoMode(600, 600), "cpp-lab-2");
	window.setFramerateLimit(60);
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "ImGui initialization failed";
		return -1;
	}

	auto spriteSize = sf::Vector2u{window.getSize().x / 2, window.getSize().y / 2};

	RClass rClassNX;
	rClassNX.Create(spriteSize, 0);
	sprites.push_back(&rClassNX);

	RClass rClassNY;
	rClassNY.Create(spriteSize, 1);
	rClassNY.setPosition(spriteSize.x, 0);
	sprites.push_back(&rClassNY);

	RClass rClassNZ;
	rClassNZ.Create(spriteSize, 2);
	rClassNZ.setPosition(0, spriteSize.y);
	sprites.push_back(&rClassNZ);

	RClass rClassNW;
	rClassNW.Create(spriteSize, 3);
	rClassNW.setPosition(spriteSize.x, spriteSize.y);
	sprites.push_back(&rClassNW);

	std::function<float(const sf::Vector2f &)> rFunction[5];

	rFunction[0] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) + std::cos(point.y); };
	rFunction[1] = [](const sf::Vector2f &point) -> float { return std::cos(point.x) * std::sin(point.y); };
	rFunction[2] = [](const sf::Vector2f &point) -> float { return std::cos(point.x + point.y); };
	rFunction[3] = [](const sf::Vector2f &point) -> float { return point.x * point.x + point.y * point.y - 200; };
	rFunction[4] = [](const sf::Vector2f &point) -> float { return std::sin(point.x) * std::cos(point.y); };

	std::function<float(const sf::Vector2f &)> complexFunction = [&rFunction](const sf::Vector2f &point) -> float
	{
		return RAnd(RAnd(ROr(RAnd(rFunction[0](point), rFunction[1](point)), rFunction[2](point)), rFunction[3](point)),
					ROr(rFunction[4](point), rFunction[0](point)));
	};

	sf::FloatRect subSpace(-10.f, -10.f, 20.f, 20.f);

	static ImVec4 firstColor(0, 0, 0, 1);
	static ImVec4 secondColor(1, 1, 1, 1);

	for (RClass *sprite : sprites)
	{
		sprite->DrawRFunc(complexFunction, subSpace);
	}

	sf::Clock deltaClock;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Controls");

		if (ImGui::ColorEdit3("First color", &firstColor.x))
		{
		}
		if (ImGui::ColorEdit3("Second color", &secondColor.x))
		{
		}

		if (ImGui::Button("Update"))
		{
			auto sfFirstColor =
				sf::Color(static_cast<sf::Uint8>(firstColor.x * 255), static_cast<sf::Uint8>(firstColor.y * 255),
						  static_cast<sf::Uint8>(firstColor.z * 255), static_cast<sf::Uint8>(firstColor.w * 255));

			auto sfSecondColor =
				sf::Color(static_cast<sf::Uint8>(secondColor.x * 255), static_cast<sf::Uint8>(secondColor.y * 255),
						  static_cast<sf::Uint8>(secondColor.z * 255), static_cast<sf::Uint8>(secondColor.w * 255));

			for (RClass *sprite : sprites)
			{
				sprite->UpdatePalette(sfFirstColor, sfSecondColor);
			}
		}


		ImGui::Text("Press 'S' to save nx,ny,nz,nw images");

		if (event.key.code == sf::Keyboard::S)
		{
			rClassNX.SaveImageToFile("images/nx.png");
			rClassNY.SaveImageToFile("images/ny.png");
			rClassNZ.SaveImageToFile("images/nz.png");
			rClassNW.SaveImageToFile("images/nw.png");
		}

		ImGui::End();

		window.clear();

		window.draw(rClassNX);
		window.draw(rClassNY);
		window.draw(rClassNZ);
		window.draw(rClassNW);

		ImGui::SFML::Render(window);

		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}