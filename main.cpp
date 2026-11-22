#include<iostream>
#include<windows.h>
#include<conio.h>
#include<random>
#include<vector>

constexpr int HEIGHT=21, WIDTH=42;
static int eaten_count = 0;

//移动光标的函数
void CursorGoTo(int x, int y) {
	COORD coord;
	coord.X = static_cast<SHORT>(x);
	coord.Y = static_cast<SHORT>(y);
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//隐藏光标，避免每次循环时光标乱跳造成屏幕闪烁。
void HideCursor() {
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hOutput, &cursorInfo); 
	cursorInfo.bVisible = FALSE;                
	SetConsoleCursorInfo(hOutput, &cursorInfo); 
}

// 食物 和 蛇 这两个类相互引用，需在食物类前面进行蛇类的声明。 此外，为避免食物引用蛇的实例的时候，出现不允许使用不完整的蛇类型，所以把食物的函数定义移动到蛇的类定义的后面。
class Snake;

//创建关于 食物 的类。
class Food {
public:
	void GenerateFoodPos(Snake& snake);
	COORD GetFoodPos() const;

private:
	COORD  food_pos{0,0};
};


//创建关于 蛇 的类
class Snake {
public:
	void GenerateInitialSnakePos();
	bool IsSnakeMoved();
	char GenerateNewDir();
	void GenerateSnakePos(Food& food);
	std::vector<COORD> GetSnakePos() const;
	COORD GetHeadPos() const;
	bool IsSnakeBodyPart(int w, int h);
	bool IsFoodEaten(Food& food, int& food_eaten_count)const;
	bool IsFenceHit() const;
	bool IsBodyHit() const;

private:
	COORD head_pos = { 0,0 };
	std::vector<COORD> snake_pos{ (0,0) };
	char dir = ' ';
	char new_dir = ' ';
};


//定义食物的相关函数

//使用mt19937( 梅森旋转Mersenne Twister 伪随机数生成引擎 )去生成随机数。
int RandInt(int lower_num, int upper_num) {
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(lower_num, upper_num);
	return dist(rng);
}

void Food::GenerateFoodPos(Snake& snake) {
	std::vector<COORD>SPos = snake.GetSnakePos();
	bool food_on_snake = true;
	//设计投放食物时避开蛇的坐标，若食物坐标在蛇身上，则重新生成食物坐标
		while (food_on_snake) {
			food_pos.X = static_cast<SHORT>(RandInt(1, WIDTH - 2));
			food_pos.Y = static_cast<SHORT>(RandInt(1, HEIGHT - 2));
			std::vector<COORD>::size_type i;
			for (i = 0; i < SPos.size(); ++i) {
				if (food_pos.X == SPos[i].X && food_pos.Y == SPos[i].Y) 
					break;
			}
			if (i>SPos.size()-1)
				break;
	   	}
}

COORD Food::GetFoodPos() const {return food_pos;}



//定义蛇的相关函数

void Snake::GenerateInitialSnakePos() {
	head_pos.X = static_cast<SHORT>(WIDTH / 2); 
	head_pos.Y = static_cast<SHORT>(HEIGHT / 2);
	snake_pos.push_back(head_pos);
};


bool Snake::IsSnakeMoved() {
	if (_kbhit()) {
		//方向键是两个字节的扩展键：_getch() 第一次返回 0 或 224，第二次才返回具体键值。
		int extended_key_prefix = _getch();
		if (extended_key_prefix == 0 || extended_key_prefix == 224) {
			int dir_KeyScanCode = _getch();
			switch (dir_KeyScanCode) {
			case 72: new_dir = 'u';  break;//按了键盘的向上方向键
			case 80: new_dir = 'd';  break; //按了键盘的向下方向键
			case 75: new_dir = 'l';  break; //按了键盘的向左方向键
			case 77: new_dir = 'r';  break; //按了键盘的向右方向键
			default: return false;
			}
			//若新方向与原方向相反，则保持原方向
			if ((dir == 'u' && new_dir == 'd') || (dir == 'd' && new_dir == 'u') || (dir == 'l' && new_dir == 'r') || (dir == 'r' && new_dir == 'l')) {
				new_dir = dir;
				return false;
			}
			else {
				dir = new_dir; // 若新方向与原方向并不是相反方向，把当前方向值更新为新方向值
				return true; //除了相反方向以外，其他方向都判断为true（上一个方向的前/左/右）
			}
		}
		else
			return false;
	}
	else
		return false;
}

char Snake::GenerateNewDir() {
	if (_kbhit()) {
		int extended_key_prefix = _getch();
		if (extended_key_prefix == 0 || extended_key_prefix == 224) {
			int dir_KeyScanCode = _getch();
			switch (dir_KeyScanCode) {
			case 72: new_dir = 'u';  break;
			case 80: new_dir = 'd';  break; 
			case 75: new_dir = 'l';   break; 
			case 77: new_dir = 'r';   break; 
			default: break;
			}

			if ((dir == 'u' && new_dir == 'd') || (dir == 'd' && new_dir == 'u') || (dir == 'l' && new_dir == 'r') || (dir == 'r' && new_dir == 'l')) 
				new_dir = dir;
			else 
				dir = new_dir; 
		}
		else
			new_dir = dir;
	}
	else
		new_dir = dir;
	return new_dir;
}

void  Snake::GenerateSnakePos(Food& food) {
	switch (new_dir) {
	case 'u':  head_pos.Y -= 1;  break;
	case 'd':  head_pos.Y += 1; break;
	case 'l':   head_pos.X -= 1; break;
	case 'r':   head_pos.X += 1; break;
	default:  break;
	}
	snake_pos.push_back(head_pos);
	COORD FPos = food.GetFoodPos();
	//若蛇还没吃到当前那份食物
	if (head_pos.X != FPos.X || head_pos.Y != FPos.Y) {
		snake_pos.erase(snake_pos.begin());
	}
}


std::vector<COORD> Snake::GetSnakePos() const { return snake_pos; }

COORD Snake::GetHeadPos() const { return head_pos; }


bool Snake::IsSnakeBodyPart(int w, int h) {
	std::vector<COORD> SPos= GetSnakePos();
			for (auto i = 0; i < SPos.size() - 1; ++i) {
				if (SPos[i].X == w && SPos[i].Y == h)
					return true;
			}
	return false;
}

bool  Snake::IsFoodEaten(Food& food, int& food_eaten_count)const {
	COORD FPos = food.GetFoodPos();
	return head_pos.X == FPos.X && head_pos.Y == FPos.Y;
}


bool Snake::IsFenceHit() const{
	return head_pos.X == 0 || head_pos.X == WIDTH - 1 || head_pos.Y == 0 || head_pos.Y == HEIGHT - 1;
}

bool Snake::IsBodyHit() const{
	for (auto i=0; i < snake_pos.size()-1;  ++i) {
		if (head_pos.X == snake_pos[i].X && head_pos.Y == snake_pos[i].Y)
			return true;
	}
	return false;
}


//游戏欢迎语、游戏说明和被吃食物计数
void GameInstructions() {
	std::cout << "Welcome to the Greedy Snake game! \n"
		<< "ARROW KEYS(↑,↓,←,→): Start the game/move the snake(except opposite direction)\n"
		<< "SNAKE HEAD( @ )\n"
		<< "FOOD( F )\n\n";
	std::cout << "COUNT OF FOOD EATEN:   " << eaten_count << "\n\n";
}


// 游戏画面渲染函数
void Render(Food& food, Snake& snake) {
	CursorGoTo(0, 5);
	COORD HPos = snake.GetHeadPos();
	COORD FPos = food.GetFoodPos();
	std::cout << "COUNT OF FOOD EATEN:   " << eaten_count << "\n\n";
	for (int h = 0; h < HEIGHT; ++h) {
		for (int w = 0; w < WIDTH; ++w) {
			//1.绘制围栏
			if (h == 0 || h == HEIGHT - 1 || w == 0 || w == WIDTH - 1) {
				//若蛇头撞入围栏，则此坐标改为绘制蛇头。
				if (HPos.X == w && HPos.Y == h) {
					std::cout << "@";
					continue;
				}
				else {
					std::cout << "#";
					continue;
				}
			}
			//2.绘制食物
			else if (FPos.X == w && FPos.Y == h) {
					std::cout << "F";
					continue;
			}
			//3.绘制蛇的身体
			else if (snake.IsSnakeBodyPart(w, h)) {
				//若蛇头咬到了身体，则此坐标改为绘制蛇头。
				if (HPos.X == w && HPos.Y == h) {
					std::cout << "@";
					continue;
				}
				else {
					std::cout << "o";
					continue;
				}
			}
			//4.绘制蛇头
			else if (HPos.X == w && HPos.Y == h) {
				std::cout << "@";
				continue;
			}
			//5.绘制空白(相应坐标没有物体)
			else
				std::cout << " ";
		}
		std::cout << "\n";
	}
	std::cout << std::endl;
}



int main() {
	HideCursor();
	Food food;
	Snake snake; 
	snake.GenerateInitialSnakePos();
	food.GenerateFoodPos(snake);
	GameInstructions();
	Render(food, snake);

	while (!snake.IsSnakeMoved()) {}

	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	bool GameOver = false;

	while (!GameOver) {

		snake.GenerateNewDir();
		snake.GenerateSnakePos(food);
		if (snake.IsFoodEaten(food, eaten_count)) {
			food.GenerateFoodPos(snake);
			++eaten_count;
		}
		Render(food, snake);
		if (snake.IsFenceHit()) {
			std::cout << "*** THE SNAKE HITS THE FENCE.  GAME OVER ! ***\n\n";
			GameOver = true;
		}
		if (snake.IsBodyHit()) {
			std::cout << "*** THE SNAKE BITES ITSELF.  GAME OVER ! *** \n\n";
			GameOver = true;
		}
		Sleep(321);
	 }	
	return 0;
}
