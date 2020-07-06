#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "fssimplewindow.h"

const int nobstacle = 5;
const int nball = 5;
const int nParticle = 30;
void PhysicalCoordToScreenCoord(int &sx, int &sy, double px, double py)
{
	sx = (int)(px*10.0);
	sy = 600 - (int)(py*10.0);
}

void DrawCircle(int cx, int cy, int rad, int fill)
{
	const double YS_PI = 3.1415927;

	if (0 != fill)
	{
		glBegin(GL_POLYGON);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}

	int i;
	for (i = 0; i<64; i++)
	{
		double angle = (double)i*YS_PI / 32.0;
		double x = (double)cx + cos(angle)*(double)rad;
		double y = (double)cy + sin(angle)*(double)rad;
		glVertex2d(x, y);
	}

	glEnd();
}

void DrawRect(int x1, int y1, int x2, int y2, int fill)
{
	if (0 != fill)
	{
		glBegin(GL_QUADS);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}

	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);

	glEnd();
}

void DrawLine(int x1, int y1, int x2, int y2)
{
	glBegin(GL_LINES);

	glVertex2i(x1, y1);
	glVertex2i(x2, y2);

	glEnd();
}

void DrawArtillery(double x, double y, double angle)
{
	int sx, sy;
	PhysicalCoordToScreenCoord(sx, sy, x, y);

	glColor3ub(0, 0, 255);
	DrawRect(sx - 5, sy - 5, sx + 5, sy + 5, 1);

	int vx, vy;
	PhysicalCoordToScreenCoord(vx, vy, x + 3.0*cos(angle), y + 3.0*sin(angle));

	DrawLine(sx, sy, vx, vy);
}

void DrawCannonBall(double x, double y)
{
	int sx, sy;
	PhysicalCoordToScreenCoord(sx, sy, x, y);

	glColor3ub(255, 0, 0);
	DrawCircle(sx, sy, 5, 1);
}


class Artillery
{
public:
	double x, y, angle;

	void Draw(double x, double y, double angle);

};

void Artillery::Draw(double x, double y, double angle)
{
	DrawArtillery(x, y, angle);
}

class CannonBall
{
public:
	double x, y, vx, vy, m, dt;
	int state;
	void Draw(double x, double y);
	void Move(double &x, double &y, double &vx, double &vy, double m, double dt);
	void Fire(double &cx, double &cy, double &vx, double &vy,
		Artillery a, double iniVel);
	void Initialize(void);

};

void CannonBall::Draw(double x, double y)
{
	DrawCannonBall(x, y);
}

void CannonBall::Move(double &x, double &y, double &vx, double &vy, double m, double dt)
{
	x += vx*dt;
	y += vy*dt;

	vy -= 9.8*dt;
}

void CannonBall::Fire(double &cx, double &cy, double &vx, double &vy,
	Artillery a, double iniVel)
{
	cx = a.x;
	cy = a.y;
	vx = iniVel*cos(a.angle);
	vy = iniVel*sin(a.angle);
}

void CannonBall::Initialize(void)
{
	state = 0;
}

class Obstacle
{
public:
	double state, x, y, w, h;

	void Draw(void);
	void Generate(void);
	int CheckHitbyCannonball(CannonBall c);
};

void Obstacle::Draw(void)
{
	glColor3ub(0, 255, 0);
	int x1, y1, x2, y2;
	PhysicalCoordToScreenCoord(x1, y1, x, y);
	PhysicalCoordToScreenCoord(x2, y2, x + w, y + h);
	DrawRect(x1, y1, x2, y2, state);
}
void Obstacle::Generate(void)
{
		state = 1;
		x = (double)(10 + rand() % 70);
		y = (double)(rand() % 60);
		w = (double)(8 + rand() % 8);
		h = (double)(8 + rand() % 8);

		if (80.0<x + w)
		{
			x = 80.0 - w;
		}
		if (60.0< y + h)
		{
			y = 60.0 - h;
		}
	
}
int Obstacle::CheckHitbyCannonball(CannonBall c)
{
	double relativeX, relativeY;
	relativeX = c.x - x;
	relativeY = c.y - y;
	if (0 <= relativeX && relativeX<w && 0 <= relativeY && relativeY < h)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

class Target
{
public:
	double state, x, y, w, h, dt;

	void Draw(void);
	void Initialize(void);
	int CheckHitbyCannonball(CannonBall c);
	void Move(void);
	void Disappear(void);

};

void Target::Draw(void)
{
	glColor3ub(255, 0, 0);
	int x1, y1, x2, y2;
	PhysicalCoordToScreenCoord(x1, y1, x, y);
	PhysicalCoordToScreenCoord(x2, y2, x + w, y + h);
	DrawRect(x1, y1, x2, y2, 1);
}

int Target::CheckHitbyCannonball(CannonBall c)
{
	double relativeX, relativeY;
	relativeX = c.x - x;
	relativeY = c.y - y;
	if (0 <= relativeX && relativeX<w && 0 <= relativeY && relativeY < h)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void Target::Initialize(void)
{
	state = 1;
}

void Target::Move(void)
{
	y -= 10 * dt;
	if (0>y)
	{
		y = 60.0;
	}
}
void Target::Disappear(void)
{
	state = 0;
}

class Explosion
{
public:
	class Point
	{
	public:
		double x, y;
	};

	class Particle
	{
	public:
		Point v, p;
	};

	int state;
	int counter;
	Particle particle[nParticle];

	void Initialize(void);
	void Move(void);
	void Draw(void);
	bool Explode(double x, double y);
};
void Explosion::Initialize(void)
{
	state = 0;
}
void Explosion::Move(void)
{
	if (0 != state)
	{
		++counter;
		//printf("%d counter\n", counter);
		if (60 <= counter)
		{
			state = 0;
		}

		for (int i = 0; i < nParticle; ++i)
		{
			particle[i].p.x += particle[i].v.x;
			particle[i].p.y += particle[i].v.y;
		}
	}
}
void Explosion::Draw(void)
{
	if (0 != state)
	{
		glPointSize(2);
		glColor3ub(255, 0, 0);
		glBegin(GL_POINTS);
		for (int i = 0; i < nParticle; ++i)
		{
			int sx, sy;
			PhysicalCoordToScreenCoord(sx, sy, particle[i].p.x, particle[i].p.y);
			glVertex2d(sx, sy);
		}
		glEnd();
	}
}
bool Explosion::Explode(double x, double y)
{
	if (0 == state)
	{
		state = 1;
		counter = 0;
		for (int i = 0; i < nParticle; ++i)
		{
			particle[i].p.x = x;
			particle[i].p.y = y;
			particle[i].v.x = double(rand() % 1001 - 500) / 50.0;
			particle[i].v.y = double(rand() % 1001 - 500) / 50.0;
		}
		return true;
	}
	return false;
}
int main()
{
	const double PI = 3.1415927;
	Artillery artillery;
	artillery.x = 5.0;
	artillery.y = 5.0;
	artillery.angle = PI / 6.0;
	CannonBall cannonball[nball];
	int nShot = 0;

	for (int i = 0; i < nball; ++i)
	{
		cannonball[i].Initialize();
	}
	
	Obstacle obstacle[nobstacle];

	Target target;
	target.Initialize();
	target.x = 75.0;
	target.y = 60.0;
	target.w = 5.0;
	target.h = 5.0;
	target.dt = 0.025;
	for (int i = 0; i < nobstacle; ++i)
	{
		obstacle[i].Generate();
	}
	
	Explosion explosion[nParticle];
	for (int i = 0; i < nParticle; ++i)
	{
		explosion[i].Initialize();
	}


	FsOpenWindow(0, 0, 800, 600, 1);
	for (;;)
	{
		FsPollDevice();

		auto key = FsInkey();
		if (FSKEY_ESC == key)
		{
			break;
		}
		if (FSKEY_SPACE == key)
		{
			
			for (int i = 0; i < nball; ++i)
			{
				if (0 == cannonball[i].state)
				{

					cannonball[i].state = 1;
					cannonball[i].Fire(cannonball[i].x, cannonball[i].y, cannonball[i].vx, cannonball[i].vy, artillery, 40.0);
					nShot++;
					break;
				}
			}
		}
		artillery.angle += PI / 180.0;
		if (PI / 2.0<artillery.angle)
		{
			artillery.angle = 0.0;
		} 
		target.Move();

		for (int i = 0; i < nParticle; ++i)
		{
			explosion[i].Move();
		}
		int flag;
		for (int i = 0; i < nball; ++i)
		{
			if (cannonball[i].state == 1)
			{
				cannonball[i].Move(cannonball[i].x, cannonball[i].y, cannonball[i].vx, cannonball[i].vy, 1.0, 0.02);
				if (cannonball[i].y < 0.0 || cannonball[i].x < 0.0 || 80.0 < cannonball[i].x)
				{
					cannonball[i].Initialize();
				}

				for (int j = 0; j < nobstacle; ++j)
				{
					if (1 == obstacle[j].state && 1 == obstacle[j].CheckHitbyCannonball(cannonball[i]))
					{

						for (int k = 0; k < nParticle; ++k)
						{

							if (true == explosion[k].Explode(obstacle[j].x, obstacle[j].y))
							{
								break;
							}
						}
						printf("Hit Obstacle!\n");
						obstacle[j].state = 0;
						cannonball[i].state = 0;




					}
				}
				if (1 == target.CheckHitbyCannonball(cannonball[i]))
				{
					flag = 1;
					for (int k = 0; k < nParticle; ++k)
					{

						if (true == explosion[k].Explode(target.x, target.y))
						{
							printf("Hit Target!\n");
							printf("You fired %d shots.\n", nShot);
							target.Disappear();
						}
					}
				}
			}
		}
		int nAlive = 0;
		if (1 == flag)
		{
			for (int k = 0; k < nParticle; ++k)
			{
				nAlive += explosion[k].state;
			}
			if (0 == nAlive)
			{
				break;
			}
		}

		
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		artillery.Draw(artillery.x, artillery.y, artillery.angle);
		for (int i = 0; i < nball; ++i)
		{
			if (cannonball[i].state == 1)
			{
				cannonball[i].Draw(cannonball[i].x, cannonball[i].y);
			}
		}

		
		for (int i = 0; i < nobstacle; ++i)
		{
			obstacle[i].Draw();
		}
		for (auto ex : explosion)
		{
			if (0 != ex.state)
			{
				ex.Draw();
			}

		}
	
		if (0 != target.state)
		{
			target.Draw();
		}
		
		

		FsSwapBuffers();
		FsSleep(25);
	}
	
	
}