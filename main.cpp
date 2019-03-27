#include <stdio.h>
#include <assert.h>

#include "DxLib.h"

const int PLAYER_MOVE_SPEED = 5;
const int BULLET_MOVE_SPEED = 5;
const int LAZER_MOVE_SPEED = 10;
const int ENEMY_MOVE_SPEED = 5;
const int WORK_SIZE = 256;
const int NUM_TASKS = 1024;
const int LAZER_X = 29;
const int LAZER_Y = 7;
const int ENEMY_X = 33;
const int ENEMY_Y = 40;

typedef struct {
	int x;
	int y;
	int img;
	int player_flag;
}PLAYER;

typedef struct {
	int lazer_x;
	int lazer_y;
	int lazer_img;
	int lazer_flag;
}LAZER;

typedef struct {
	int enemy_x;
	int enemy_y;
	int enemy_img;
	int enemy_x_flag;
	int enemy_y_flag;
	int enemy_active_end;
	int flag;
	int enemy_flag;
}ENEMY;

typedef struct {
	int left;
	int right;
	int top;
	int bottom;
}HIT;

struct TASK {

	void(*Func)(TASK* task);

	TASK* Prev;
	TASK* Next;

	PLAYER player;
	LAZER lazer;
	ENEMY enemy;

	HIT Hit;

	char Work[WORK_SIZE];
};

typedef void(*FUNC)(TASK* task);

TASK* ActiveTask;
TASK* FreeTask;

char g_key[256];

int hantei[15][20] = {
	{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 },
};

void InitTaskList() {

	TASK* task = new TASK[NUM_TASKS + 2];

	ActiveTask = &task[0];
	ActiveTask->Prev = ActiveTask->Next = ActiveTask;

	FreeTask = &task[1];
	for (int i = 1; i < NUM_TASKS + 1; i++)
		task[i].Next = &task[i + 1];
	task[NUM_TASKS + 1].Next = FreeTask;
}

void RunTask() {
	for (TASK *task = ActiveTask->Next, *next;
	next = task->Next, task != ActiveTask; task = next)
		(*task->Func)(task);
}

TASK* CreateTask(FUNC func) {

	if (FreeTask->Next == FreeTask) return NULL;

	TASK* task = FreeTask->Next;
	FreeTask->Next = task->Next;

	task->Func = func;
	task->Prev = ActiveTask->Prev;
	task->Next = ActiveTask;

	task->Prev->Next = task;
	task->Next->Prev = task;

	return task;
}

void DeleteTask(TASK* task) {

	task->Prev->Next = task->Next;
	task->Next->Prev = task->Prev;

	task->Next = FreeTask->Next;
	FreeTask->Next = task;
}

const int is_collision(TASK* task0) {

	int l0 = task0->enemy.enemy_x + task0->Hit.left;
	int r0 = task0->enemy.enemy_x + task0->Hit.right;
	int t0 = task0->enemy.enemy_y + task0->Hit.top;
	int b0 = task0->enemy.enemy_y + task0->Hit.bottom;

	for (TASK* task1 = ActiveTask->Next, *next; next = task1->Next, task1 != ActiveTask; task1 = next) {

		if (task1->lazer.lazer_flag) {
			int l1 = task1->lazer.lazer_x + task1->Hit.left;
			int r1 = task1->lazer.lazer_x + task1->Hit.right;
			int t1 = task1->lazer.lazer_y + task1->Hit.top;
			int b1 = task1->lazer.lazer_y + task1->Hit.bottom;

			if (l0 < r1 && l1 < r0 && t0 < b1 && t1 < b0) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

void player_control(TASK* task) {

	DrawGraph(task->player.x, task->player.y, task->player.img, TRUE);

	if (g_key[KEY_INPUT_RIGHT] == 1) {
		task->player.x += PLAYER_MOVE_SPEED;
	}
	else if (g_key[KEY_INPUT_LEFT] == 1) {
		task->player.x -= PLAYER_MOVE_SPEED;
	}
	else if (g_key[KEY_INPUT_UP] == 1) {
		task->player.y -= PLAYER_MOVE_SPEED;
	}
	else if (g_key[KEY_INPUT_DOWN] == 1) {
		task->player.y += PLAYER_MOVE_SPEED;
	}
}

void create_player(const int x, const int y, const int image) {
	TASK* task = CreateTask(player_control);

	if (!task) return;

	task->player.x = x;
	task->player.y = y;
	task->player.img = image;
	task->player.player_flag = TRUE;
	task->lazer.lazer_flag = FALSE;
	task->enemy.enemy_flag = FALSE;
}

void lazer_shot(TASK* task) {

	DrawGraph(task->lazer.lazer_x, task->lazer.lazer_y, task->lazer.lazer_img, TRUE);

	if (task->lazer.lazer_x > 640) {
		DeleteTask(task);
		return;
	}

	task->lazer.lazer_x += LAZER_MOVE_SPEED;
}

void create_lazer(const int lazer_image) {
	TASK* task = CreateTask(lazer_shot);

	if (!task) return;

	for (TASK* task1 = ActiveTask->Next, *next; next = task1->Next, task1 != ActiveTask; task1 = next) {
		if (task1->player.player_flag) {
			task->lazer.lazer_x = task1->player.x + 56;
			task->lazer.lazer_y = task1->player.y + 8;
			task->Hit.left = 0;
			task->Hit.right = LAZER_X;
			task->Hit.top = 0;
			task->Hit.bottom = LAZER_Y;
			task->lazer.lazer_img = lazer_image;
			task->lazer.lazer_flag = TRUE;
			task->player.player_flag = FALSE;
			task->enemy.enemy_flag = FALSE;
		}
	}
}

void enemy_move(TASK* task) {

	DrawGraph(task->enemy.enemy_x, task->enemy.enemy_y, task->enemy.enemy_img, TRUE);

	if (task->enemy.flag || is_collision(task)) {
		DeleteTask(task);
		return;
	}

	if (task->enemy.enemy_x > 500 && task->enemy.enemy_x_flag) {
		task->enemy.enemy_x -= ENEMY_MOVE_SPEED;

		if (task->enemy.enemy_x <= 500) {
			task->enemy.enemy_x_flag = FALSE;
			task->enemy.enemy_y_flag = TRUE;
		}
	}
	else if (task->enemy.enemy_y < 100 && task->enemy.enemy_y_flag) {
		task->enemy.enemy_y += ENEMY_MOVE_SPEED;

		if (task->enemy.enemy_y >= 100) {
			task->enemy.enemy_x_flag = FALSE;
			task->enemy.enemy_y_flag = FALSE;
			task->enemy.enemy_active_end = TRUE;
		}
	}
	else if (task->enemy.enemy_x < 640 && task->enemy.enemy_active_end) {
		task->enemy.enemy_x += ENEMY_MOVE_SPEED;
	}

	if (task->enemy.enemy_x > 640) {
		task->enemy.flag = TRUE;
	}
}

void create_enemy(const int enemy_image) {
	TASK* task = CreateTask(enemy_move);

	if (!task) return;

	task->enemy.enemy_x = 640;
	task->enemy.enemy_y = 50;
	task->Hit.left = 0;
	task->Hit.right = ENEMY_X;
	task->Hit.top = 0;
	task->Hit.bottom = ENEMY_Y;
	task->enemy.enemy_img = enemy_image;
	task->enemy.enemy_x_flag = TRUE;
	task->enemy.enemy_y_flag = FALSE;
	task->enemy.enemy_active_end = FALSE;
	task->enemy.flag = FALSE;
	task->player.player_flag = FALSE;
	task->lazer.lazer_flag = FALSE;
	task->enemy.enemy_flag = TRUE;
}

void draw_background(const int map_image[]) {
	int i, j = 0;

	for (i = 0; i < 15; i++) {
		for (j = 0; j < 20; j++) {
			if (hantei[i][j] == 1) {
				DrawGraph(j * 32, i * 32, map_image[33], TRUE);
			}
			else if (hantei[i][j] == 2) {
				DrawGraph(j * 32, i * 32, map_image[39], TRUE);
			}
			else {
				DrawGraph(j * 32, i * 32, map_image[5], TRUE);
			}
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ChangeWindowMode(TRUE);
	DxLib_Init();
	SetDrawScreen(DX_SCREEN_BACK);
	InitTaskList();

	int player_image;
	int lazer_image;
	int enemy1_1_image;
	int map_image[54];
	int lazer_cnt = 0;
	int lazer_wait = FALSE;
	int enemy_cnt = 0;
	int enemy_wait = FALSE;

	player_image = LoadGraph("player1_jiki.png");
	lazer_image = LoadGraph("player1_shot.png");
	enemy1_1_image = LoadGraph("enemy1_1.png");
	LoadDivGraph("mapchip1.png", 54, 6, 9, 32, 32, map_image);

	create_player(50, 50, player_image);

	while (!ScreenFlip() && !ProcessMessage() && !GetHitKeyStateAll(g_key) && !ClearDrawScreen()) {

		draw_background(map_image);

		if (g_key[KEY_INPUT_S] && !lazer_wait) {
			create_lazer(lazer_image);
			lazer_wait = TRUE;
		}

		if (lazer_cnt >= 10) {
			lazer_wait = FALSE;
			lazer_cnt = 0;
		}

		if (lazer_wait) {
			lazer_cnt++;
		}

		if (!enemy_wait) {
			create_enemy(enemy1_1_image);
			enemy_wait = TRUE;
		}

		if (enemy_cnt >= 15) {
			enemy_wait = FALSE;
			enemy_cnt = 0;
		}

		if (enemy_wait) {
			enemy_cnt++;
		}

		RunTask();
	}

	DxLib_End();

	return 0;
}