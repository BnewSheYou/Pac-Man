#include<iostream>
#include<easyx.h>
#include<list>


bool IsOverLay(RECT rect, POINT pos) {
	if (rect.left <= pos.x && pos.x <= rect.right && rect.top <= pos.y && pos.y <= rect.bottom) {
		
			return true;
	}
	return false;
}
//判断鼠标是否在矩形内

bool IsOverLay(RECT rect1, RECT rect2) {
	if (rect1.left < rect2.right && rect2.left < rect1.right && rect1.top < rect2.bottom && rect2.top < rect1.bottom)
	{
		return true;
	}
	return false;
}
//判断两个矩形是否重叠

bool ListToFile(const std::list<RECT>& rectlist,const char* filename )//从链表写入文件
{
	//打开文件
	FILE* fp = fopen(filename, "wb");
	//写入文件
    if (fp == nullptr) return false;
	for (auto& rect : rectlist) {
		fwrite(&rect, sizeof(RECT), 1, fp);
		printf("rectlist size: %zu\n", rectlist.size());
	}
	fclose(fp);
	return true;
}
bool FileToList(std::list<RECT>& rectlist, const char* filename)//从文件读取
{	FILE* fp = fopen(filename, "rb");
	if (fp == nullptr) return false;
	RECT rect;
	while (true) {
		int ret = fread(&rect, sizeof(RECT), 1, fp);
		if (ret <= 0)break;
		rectlist.push_back(rect);
	}
	fclose(fp);
	return true;
}

struct Candy {
	POINT pos;
	bool eaten = false;
};

int main() {
	initgraph(451, 492, EX_SHOWCONSOLE);
	//加载背景
	IMAGE img;
	loadimage(&img, "res/Background.png");
	
	//加载小人
	IMAGE player[4];
	loadimage(&player[0], "res/Player_00.png");
	loadimage(&player[1], "res/Player_01.png");
	loadimage(&player[2], "res/Player_02.png");
	loadimage(&player[3], "res/Player_03.png");

	std::list<Candy> candyList;

	IMAGE candyImg;
	loadimage(&candyImg, "res/Candy.png"); // 路径根据你的实际文
	
	bool isMouseDown = false;
	POINT tempRectBegin = { 0,0 };
	POINT tempRectEnd = { 0 ,0 };

	std::list<RECT> m_RectList;
	FileToList(m_RectList, "rect.ce");
	//糖豆位置
	int candyCols = 11; // 列数
	int candyRows = 12; // 行数
	int totalWidth = 451;
	int totalHeight = 492;
	int xMargin = totalWidth / (candyCols + 1);
	int yMargin = totalHeight / (candyRows + 1);

	for (int row = 0; row < candyRows; ++row) {
		int y = yMargin * (row + 1);
		for (int col = 0; col < candyCols; ++col) {
			int x = xMargin * (col + 1);
			POINT pt = { x, y };
			bool overlap = false;
			for (auto& rect : m_RectList) {
				if (IsOverLay(rect, pt)) {
					overlap = true;
					break;
				}
			}
			if (!overlap) {
				candyList.push_back({ pt, false });
			}
		}
	}
	//小人位置
	float playerX = 15, playerY= 15, playerW = player->getwidth(), playerH =player->getheight();
	//小人方向
	INT direction = 3;
	//分数
	int score = 0;
	//幽灵
	IMAGE ghostImg;
	struct Ghost {
		float x, y;
		float speed;
	};
	loadimage(&ghostImg, "res/Ghost.png");
	// 幽灵初始化，避免贴边和障碍
	float ghostInitX = totalWidth / 2.0f;
	float ghostInitY = totalHeight / 2.0f;
	RECT ghostInitRect = { (long)ghostInitX, (long)ghostInitY,
						   (long)ghostInitX + ghostImg.getwidth(),
						   (long)ghostInitY + ghostImg.getheight() };
	bool ghostOverlap = false;
	for (auto& rect : m_RectList) {
		if (IsOverLay(rect, ghostInitRect)) {
			ghostOverlap = true;
			break;
		}
	}
	// 如果中心有障碍，则向右下偏移，直到找到安全点
	while (ghostOverlap) {
		ghostInitX += 10;
		ghostInitY += 10;
		if (ghostInitX + ghostImg.getwidth() > totalWidth) ghostInitX = 30;
		if (ghostInitY + ghostImg.getheight() > totalHeight) ghostInitY = 30;
		ghostInitRect = { (long)ghostInitX, (long)ghostInitY,
						  (long)ghostInitX + ghostImg.getwidth(),
						  (long)ghostInitY + ghostImg.getheight() };
		ghostOverlap = false;
		for (auto& rect : m_RectList) {
			if (IsOverLay(rect, ghostInitRect)) {
				ghostOverlap = true;
				break;
			}
		}
	}
	Ghost ghost = { ghostInitX, ghostInitY, 0.04f };


	ExMessage msg;
	BeginBatchDraw();//双缓冲
	while (true) {
		if (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN) {
				tempRectBegin.x = msg.x;
				tempRectBegin.y = msg.y;
				isMouseDown = true;
			}
			else if (msg.message == WM_MOUSEMOVE) {
				tempRectEnd.x = msg.x;
				tempRectEnd.y = msg.y;
			}
			else if (msg.message == WM_LBUTTONUP)
			{
				isMouseDown = false;
				RECT Rect;
				if (tempRectBegin.x > tempRectEnd.x)
				{
					Rect.left = tempRectEnd.x;
					Rect.right = tempRectBegin.x;
				}
				else
				{
					Rect.left = tempRectBegin.x;
					Rect.right = tempRectEnd.x;
				}
				if (tempRectBegin.y > tempRectEnd.y) {
					Rect.top = tempRectEnd.y;
					Rect.bottom = tempRectBegin.y;
				}
				else
				{
					Rect.top = tempRectBegin.y;
					Rect.bottom = tempRectEnd.y;
				}

				//判断矩形的面积，如果面积太小，就会产生一个看不见的矩形阻挡player移动
				if ((Rect.right - Rect.left)* (Rect.bottom - Rect.top) > 9)
				{
					printf("(%d,%d)(%d,%d)\n", Rect.left, Rect.top, Rect.right, Rect.bottom);
					m_RectList.push_back(Rect);
					ListToFile(m_RectList, "rect.ce");
				}
			}
			else if (msg.message == WM_RBUTTONDOWN) {
				POINT pos = { msg.x, msg.y };
				std::list<RECT>::iterator iterator = m_RectList.begin();
				for (auto it = m_RectList.begin(); it != m_RectList.end(); ) {
					if (IsOverLay(*it, pos)) {
						it = m_RectList.erase(it); // erase返回下一个有效迭代器
					}
					else {
						++it;
					}
				}
				ListToFile(m_RectList, "rect.ce");
			}
		}
		FLOAT speed = 0.05f;
		if (GetAsyncKeyState('W')) {
			playerY -= speed;
			direction = 0;
			RECT playerRect = { (long)playerX,(long)playerY,(long)playerX + (long)playerW,(long)playerY + (long)playerH };
			for (auto& rect : m_RectList) {
				if (IsOverLay(rect, playerRect))
				{
					playerY += speed;
					break;
				}

			}
		}
		if (GetAsyncKeyState('S')) {
			playerY += speed;
			direction = 1;
			RECT playerRect = { (long)playerX,(long)playerY,(long)playerX + (long)playerW,(long)playerY + (long)playerH };
			for (auto& rect : m_RectList) {
				if (IsOverLay(rect, playerRect))
				{
					playerY -= speed;
					break;
				}

			}
		}
		if(GetAsyncKeyState('A')) {
			playerX -= speed;
			direction = 2;
			RECT playerRect = { (long)playerX,(long)playerY,(long)playerX + (long)playerW,(long)playerY + (long)playerH };
			for (auto& rect : m_RectList) {
				if (IsOverLay(rect, playerRect))
				{
					playerX += speed;
					break;
				}

			}
		}
		if (GetAsyncKeyState('D')) {
			playerX += speed;
			direction = 3;
			RECT playerRect = { (long)playerX,(long)playerY,(long)playerX + (long)playerW,(long)playerY + (long)playerH };
			for (auto& rect : m_RectList) {
				if (IsOverLay(rect, playerRect))
				{
					playerX -= speed;
					break;
				}

			}
		}
		// 幽灵追击player
		float dx = playerX - ghost.x;
		float dy = playerY - ghost.y;
		float dist = sqrt(dx * dx + dy * dy);
		if (dist > 1e-3) {
			float step = ghost.speed;
			float moveX = step * dx / dist;
			float moveY = step * dy / dist;
			// 预判下一步是否撞墙
			RECT ghostRect = { (long)(ghost.x + moveX), (long)(ghost.y + moveY),
							   (long)(ghost.x + moveX) + ghostImg.getwidth(),
							   (long)(ghost.y + moveY) + ghostImg.getheight() };
			bool collide = false;
			for (auto& rect : m_RectList) {
				if (IsOverLay(rect, ghostRect)) {
					collide = true;
					break;
				}
			}
			if (!collide) {
				ghost.x += moveX;
				ghost.y += moveY;
			}
		}
		cleardevice();//清空屏幕
		
		putimage(0, 0, &img);
        putimage(playerX, playerY, &player[direction]);

		//player与糖豆碰撞检测与消除
		RECT playerRect = { (long)playerX,(long)playerY,(long)playerX + (long)playerW,(long)playerY + (long)playerH };
		for (auto& candy : candyList) {
			if (!candy.eaten && IsOverLay(playerRect, candy.pos)) {
				candy.eaten = true;
				score += 1;// 可加分数等逻辑
			}
		}
		//幽灵与player碰撞检测
		RECT ghostRect = { (long)ghost.x, (long)ghost.y, (long)ghost.x + ghostImg.getwidth(), (long)ghost.y + ghostImg.getheight() };
		if (IsOverLay(playerRect, ghostRect)) {
			settextcolor(RED);
			settextstyle(36, 0, "Consolas");
			outtextxy(totalWidth / 2 - 80, totalHeight / 2, "Game Over!");
			FlushBatchDraw();
			Sleep(2000);
			break;
		}
		//绘制糖豆
		for (auto& candy : candyList) {
			if (!candy.eaten) {
				// 让糖豆图片中心对齐糖豆坐标
				int imgW = candyImg.getwidth();
				int imgH = candyImg.getheight();
				putimage(candy.pos.x - imgW / 2, candy.pos.y - imgH / 2, &candyImg);
			}
		}
		//绘制分数
		setbkmode(TRANSPARENT); // 让文字背景透明
		settextcolor(RED);
		settextstyle(24, 0, "Consolas");
		char scoreText[32];
		sprintf(scoreText, "Score: %d", score);
		outtextxy(10, 10, scoreText);
		//绘制幽灵
		putimage(ghost.x, ghost.y, &ghostImg);
		//将player放到一个RECT中
		rectangle(playerX, playerY, playerX + player->getwidth(), playerY + player->getheight());
		for (auto& rect: m_RectList) {
			rectangle(rect.left, rect.top, rect.right, rect.bottom);
		}
		if (isMouseDown)
		{
		rectangle(tempRectBegin.x, tempRectBegin.y, tempRectEnd.x, tempRectEnd.y);
		}
		FlushBatchDraw();
	}

}