#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;
/*
0,0 j����>
 i
 |
\|/
*/

#define INT_MAX 2147483647
#define INT_MIN -2147483647

#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2

#define TEMP 5
#define MAX_DEPTH 3

int DEFENCE_WEIGHT = 15;
int ATTACK_WEIGHT = 10;

#define REAR 25

typedef int BOOL;
#define TRUE 1
#define FALSE 0
#define START "START"
#define PLACE "PLACE"
#define TURN "TURN"
#define END "END"

// ����
enum { UP, DOWN, LEFT, RIGHT, UL, DR, DL, UR };//��ֵ�ֱ��Ӧ����ķ�������
int dir_j[8] = { 0,0,-1,1,-1,1,-1,1 };
int dir_i[8] = { -1,1,0,0,-1,1,1,-1 };
#define U2D 0//�����ĸ�����ֵ��ֵ+1������ָ�������������ߣ�
#define L2R 2
#define UL2DR 4
#define DL2UR 6

//����
#define NOTHREAT 0
#define WIN5 1
#define ALIVE4 2
#define RUSH4 3
#define ALIVE3 4
#define JUMP3 5
#define SLEEP3 6
#define ALIVE2 7
#define JUMP2 8
#define B_JUMP2 9
#define SLEEP2 10
#define L_RUSH4 11//���º��г����ĳ���

//����
#define L1 10000
#define L2 9000
#define L3 8000
#define L4 7500
#define L5 1000
#define L6 900
#define L7 100
#define L8 50
#define L9 10
#define L10 0

struct Coord {//����ṹ��
    int x;
    int y;
};

struct Command
{
    int x;
    int y;
};
char board[BOARD_SIZE][BOARD_SIZE] = { 0 };

int myFlag;
int enemyFlag;//����˫��ִ�ӵ���ɫ

int Round = 0;

int pi = BOARD_SIZE / 2, pj = BOARD_SIZE / 2;

bool is_in_boundary(Coord coord) {
    return coord.x >= 0 && coord.x < BOARD_SIZE && coord.y >= 0 && coord.y < BOARD_SIZE;
}

bool flagcmp(int chess, int flag) {
    if (chess % TEMP == flag)
        return true;
    else
        return false;
}

struct Node {//���߽��
    int x;
    int y;
    int flag;
    int max_min;//1Ϊmax��0λmin���
    int ex_value;//������ļ�ֵ
    int value;//value����flagһ�����������õ��������
    Coord desi;
    vector <Node> children;
    Node(int xx, int yy, int f, int is_max) :x(xx), y(yy), flag(f), value(0), max_min(is_max), desi({ -1,-1 })
    {
        if (is_max == 1) ex_value = INT_MAX; else ex_value = INT_MIN;
    }
};

class Situation {
    int Chess_type[12];//��0-11����ʮһ�ֲ�ͬ����
    int value;

public:
    Situation() { for (int i = 0; i < 12; i++) Chess_type[i] = 0; value = 0; }
    void print() { for (int i = 0; i < 11; i++) printf("%d ", Chess_type[i]); printf("\n"); }
    void calculate_edge(int* p, int size, Coord coord, int flag, int dir);
    void mark_coord(Coord coord, int dir) { if (board[coord.x][coord.y] < (dir / 2 + 1) * TEMP) board[coord.x][coord.y] += TEMP; }//���ʹ�ù��ĵ�
    void mark_clear();
    void evaluate_type(Coord coord, int dir);//ֻ��������ǿ���δ����ǵ�λ��
    void evaluate_all(int flag);
    int cal_value(bool is_enemy);
};

int Situation::cal_value(bool is_enemy) {
    /*//����
#define NOTHREAT 0
#define WIN5 1
#define ALIVE4 2
#define RUSH4 3
#define ALIVE3 4
#define JUMP3 5
#define SLEEP3 6
#define ALIVE2 7
#define JUMP2 8
#define B_JUMP2 9
#define SLEEP2 10
#define L_RUSH4 11

//����
#define L1 10000
#define L2 9000
#define L3 8000
#define L4 7500
#define L5 1000
#define L6 900
#define L7 100
#define L8 50
#define L9 10
#define L10 0
*/
    if (Chess_type[WIN5] > 0) {
        value += L1;
        return value;
    }
    if (Chess_type[ALIVE4] > 0 || Chess_type[RUSH4] >= 2) {//��4��˫����
        value += L2;
        return value;
    }
    if ((Chess_type[RUSH4] > 0 && Chess_type[ALIVE3] > 0) ||
        (Chess_type[RUSH4] > 0 && Chess_type[JUMP3] > 0) ||
        (Chess_type[L_RUSH4] > 0 && Chess_type[ALIVE3] > 0) ||
        (Chess_type[L_RUSH4] > 0 && Chess_type[JUMP3] > 0)) {//���Ļ���/����
        if (is_enemy)
            value += L3 + 500;
        else
            value += L3;
        return value;
    }
    if ((Chess_type[ALIVE3] >= 2 || Chess_type[JUMP3] >= 2) ||
        (Chess_type[JUMP3] > 0 && Chess_type[ALIVE3] > 0)) {//˫����������������
        if (is_enemy)
            value += L3;
        else
        value += L4;
        return value;
    }
    if (Chess_type[RUSH4] > 0 || Chess_type[ALIVE3] > 0 || Chess_type[JUMP3] > 0) {//���Ļ����������������������
        if (is_enemy)
            value += L1;
        else
            value += L5;
    }
    if (Chess_type[L_RUSH4] > 0) {// �ͼ����ģ��������Ҳ���������
        if (is_enemy)
            value += L1;
        else
            value += L6;
    }
    if (is_enemy && myFlag == BLACK) {
        value += Chess_type[SLEEP3] * L7 / 5;
        value += ((Chess_type[ALIVE2] + Chess_type[JUMP2] + Chess_type[B_JUMP2]) * L8 / 5);
        value += Chess_type[SLEEP2] * L9 / 5;
        value += Chess_type[NOTHREAT] * L10 / 5;
    }
    else if (is_enemy && myFlag == WHITE) {
        value += Chess_type[SLEEP3] * L7 / 5;
        value += ((Chess_type[ALIVE2] + Chess_type[JUMP2] + Chess_type[B_JUMP2]) * L8 / 5);
        value += Chess_type[SLEEP2] * L9 / 5;
        value += Chess_type[NOTHREAT] * L10 / 5;
    }

    else {
        value += Chess_type[SLEEP3] * L7;
        value += ((Chess_type[ALIVE2] + Chess_type[JUMP2] + Chess_type[B_JUMP2]) * L8);
        value += Chess_type[SLEEP2] * L9;
        value += Chess_type[NOTHREAT] * L10;
    }
    return value;
}

void Situation::mark_clear() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] >= TEMP)
                board[i][j] = board[i][j] % TEMP;
        }
    }
}

void Situation::calculate_edge(int* p, int size, Coord coord, int flag, int dir) {//�������м��������¡����������,�����б��
    for (int i = 0; i < size; i++) {
        if (is_in_boundary(coord)) {
            if (flagcmp(board[coord.x][coord.y], flag)) {//�ҷ�
                p[i] = flag;
                //used_coord.insert(coord);//������ʹ�õ������У���Ϊ�ṹ������
            }
            else if (flagcmp(board[coord.x][coord.y], 3 - flag)) {//�з�
                p[i] = 3 - flag;
                break;//������ֱ���ס�Ͳ����жϽ�������������
            }
            //����Ϊ�գ����ı�ֵ
        }
        else {
            p[i] = 3 - flag;
            break;//������ֱ���ס�Ͳ����жϽ�������������
        }
        coord.x += dir_j[dir], coord.y += dir_i[dir];
    }
}

void Situation::evaluate_type(Coord coord, int dir) {
    int flag = board[coord.x][coord.y] % TEMP;
    mark_coord(coord, dir);
    int count = 1;
    //if (flag == EMPTY)
        //return;
    int x, xx, y, yy;//x��y������ֱ���ϵĵ�һ�������ϱ���������xx��yyΪ�ڶ��������ϵģ�����U2D��һ��ڶ�����ֱ����UP��DOWN
    x = coord.x + dir_j[dir], y = coord.y + dir_i[dir];
    xx = coord.x + dir_j[dir + 1], yy = coord.y + dir_i[dir + 1];
    for (; is_in_boundary({ x,y }) && flagcmp(board[x][y], flag); x += dir_j[dir], y += dir_i[dir]) {
        mark_coord({ x,y }, dir);
        count++;
    }
    for (; is_in_boundary({ xx,yy }) && flagcmp(board[xx][yy], flag); xx += dir_j[dir + 1], yy += dir_i[dir + 1]) {
        mark_coord({ xx,yy }, dir);//�������ӣ���¼λ�ò�����������һ
        count++;
    }
    if (count >= 5) {
        Chess_type[WIN5]++;
        return;
    }
    int _xy[5] = { 0 };
    int _xxyy[5] = { 0 };//����÷����ϵ�����5����
    calculate_edge(_xy, 5, { x,y }, flag, dir);
    calculate_edge(_xxyy, 5, { xx,yy }, flag, dir + 1);

    switch (count) {
    case 4://�Ŀ�����
        if (is_in_boundary({ x,y }) && is_in_boundary({ xx,yy })) {
            if (board[x][y] == EMPTY && board[xx][yy] == EMPTY)//����λ��Ϊ�գ�Ϊ����
                Chess_type[ALIVE4]++;
            else if (board[x][y] == EMPTY || board[xx][yy] == EMPTY)//��һ�߱��Է���ס��Ϊ����
                Chess_type[L_RUSH4]++;
            else
                Chess_type[NOTHREAT]++;//���߶���������û����в
        }
        else if (!is_in_boundary({ x,y }) && is_in_boundary({ xx,yy })) {//��һ�߳���
            if (board[xx][yy] == EMPTY)//��һ�߱���ס��Ϊ����
                Chess_type[L_RUSH4]++;
            else
                Chess_type[NOTHREAT]++;//���߶���������û����в
        }
        else if (is_in_boundary({ x,y }) && !is_in_boundary({ xx,yy })) {//��һ�߳���
            if (board[x][y] == EMPTY)//��һ�߱���ס��Ϊ����
                Chess_type[L_RUSH4]++;
            else
                Chess_type[NOTHREAT]++;//���߶���������û����в
        }
        else
            Chess_type[NOTHREAT]++;//���߶���������û����в
        break;
    case 3://��������

        if (_xy[0] == EMPTY && _xxyy[0] == EMPTY) {
            if (_xy[1] == 3 - flag && _xxyy[1] == 3 - flag)
                Chess_type[SLEEP3]++;
            else if (_xy[1] == flag && _xxyy[1] == flag) {//����λ�ö��Ǽ��������ӵĻ���ֱ�Ӽ�����RUSH4,�Ѿ�����ȡʤ
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);
                Chess_type[L_RUSH4] += 2;
            }
            else if (_xy[1] == flag) {
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);
                if (_xxyy[1] == 3 - flag)
                    Chess_type[L_RUSH4]++;
                else
                    Chess_type[RUSH4]++;
            }
            else if (_xxyy[1] == flag) {
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);
                if (_xy[1] == 3 - flag)
                    Chess_type[L_RUSH4]++;
                else
                    Chess_type[RUSH4]++;
            }
            else if (_xy[1] == EMPTY || _xxyy[1] == EMPTY)//�ж���������ʡ��
                Chess_type[ALIVE3]++;
        }

        else if (_xy[0] == 3 - flag && _xxyy[0] == 3 - flag)
            Chess_type[NOTHREAT]++;

        else if (_xy[0] == 3 - flag) {//����һ��û�б���
            if (_xxyy[1] == flag) {
                Chess_type[L_RUSH4]++;
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);
            }
            else if (_xxyy[1] == EMPTY)
                Chess_type[SLEEP3]++;
            //��������в
        }
        else if (_xxyy[0] == 3 - flag) {//����һ��û�б���
            if (_xy[1] == flag) {
                Chess_type[L_RUSH4]++;
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);
            }
            else if (_xy[1] == EMPTY)
                Chess_type[SLEEP3]++;
            //��������в
        }
        break;
    case 2://��������

        /*head X X 0' * * 0' X X */
        if (_xy[0] == EMPTY && _xxyy[0] == EMPTY) {
            if (_xy[1] == EMPTY && _xxyy[1] == EMPTY) {
                /*sub X 0' 0 * * 0 0' X */

                if (_xy[2] == flag && _xxyy[2] == flag) {//����λ�ö��Ǽ��������ӵĻ���ֱ�Ӽ�����SLEEP3
                    Chess_type[SLEEP3] += 2;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                }
                else if (_xy[2] == flag) {/*sub2 *' 0 0 * * 0 0 X */
                    Chess_type[SLEEP3]++;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                }
                else if (_xxyy[2] == flag) {/*sub2 X 0 0 * * 0 0 *' */
                    Chess_type[SLEEP3]++;
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                }
                else/*sub2 X' 0 0 * * 0 0 X' */
                    Chess_type[ALIVE2]++;
            }
            else if (_xy[1] == 3 - flag && _xxyy[1] == 3 - flag)
                /*sub X 1' 0 * * 0 1' X */
                Chess_type[NOTHREAT]++;

            else if (_xy[1] == 3 - flag) {
                /*sub X 1' 0 * * 0 0 X */

                if (_xxyy[2] == EMPTY)/*sub2 X 1 0 * * 0 0 0' */
                    Chess_type[ALIVE2]++;
                else if (_xxyy[2] == flag) {/*sub2 X 1 0 * * 0 0 *' */
                    Chess_type[SLEEP3]++;
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                }
                else/*sub2 X 1 0 * * 0 0 1' */
                    Chess_type[SLEEP2]++;
            }
            else if (_xxyy[1] == 3 - flag) {
                /*sub X 0 0 * * 0 1' X */

                if (_xy[2] == EMPTY)/*sub2 0' 0 0 * * 0 1 X */
                    Chess_type[ALIVE2]++;
                else if (_xy[2] == flag) {/*sub2 *' 0 0 * * 0 1 X */
                    Chess_type[SLEEP3]++;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                }
                else/*sub2 1' 0 0 * * 0 1 X */
                    Chess_type[SLEEP2]++;
            }

            /*else if (_xy[1] == flag && _xxyy[1] == flag) {
                /*sub X *' 0 * * 0 *' X */
                /*   mark_coord({x + 1 * dir_j[dir],y + 1 * dir_i[dir]});
                   mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] });

                   if (_xy[2] == flag && _xxyy[2] == flag) {
                       mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] });
                       mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] });
                       Chess_type[ALIVE4]++;
                   }
                   else if (_xy[2] == 3 - flag && _xxyy[2] == 3 - flag) {

                   }
               }*/
            else if (_xy[1] == flag && _xxyy[1] == flag && _xy[2] == flag && _xxyy[2] == flag) {
                /*sub * *' 0 * * 0 *' * */
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);

                mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                Chess_type[ALIVE4]++;

            }

            else if (_xy[1] == flag) {
                /*sub X *' 0 * * 0 0 X */
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);

                if (_xy[2] == EMPTY)/*sub2 0 * 0 * * 0 0 X */
                    Chess_type[ALIVE3]++;
                else if (_xy[2] == flag) {/*sub2 * * 0 * * 0 0 X */
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                    if (_xy[3] == 3 - flag)
                        Chess_type[L_RUSH4]++;
                    else if (_xy[3] == EMPTY)
                        Chess_type[ALIVE4]++;
                }
                else if (_xy[2] == 3 - flag)/*sub2 1 * 0 * * 0 0 X */
                    Chess_type[SLEEP3]++;
            }
            else if (_xxyy[1] == flag) {
                /*sub X 0 0 * * 0 * X */ //����
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);

                if (_xxyy[2] == EMPTY)
                    Chess_type[ALIVE3]++;
                else if (_xxyy[2] == flag) {
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                    if (_xxyy[3] == 3 - flag)
                        Chess_type[L_RUSH4]++;
                    else if (_xxyy[3] == EMPTY)
                        Chess_type[ALIVE4]++;
                }
                else if (_xxyy[2] == 3 - flag)
                    Chess_type[SLEEP3]++;
            }

        }

        /*head X X 1' * * 1' X X */
        else if (_xy[0] == 3 - flag && _xxyy[0] == 3 - flag)
            Chess_type[NOTHREAT]++;

        /*head X X 1' * * 0' X X */
        else if (_xy[0] == 3 - flag) {//����һ��û�б���

            if (_xxyy[1] == EMPTY) {
                /*sub X X 1 * * 0 0' X */

                if (_xxyy[2] == EMPTY)/*sub2 X X 1 * * 0 0 0' */
                    Chess_type[SLEEP2]++;
                else if (_xxyy[2] == flag) {/*sub2 X X 1 * * 0 0 *' */
                    Chess_type[SLEEP3]++;
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                }
                else/*sub2 X X 1 * * 0 0 1' */
                    Chess_type[NOTHREAT]++;
            }
            else if (_xxyy[1] == flag) {
                /*sub X X 1 * * 0 *' X */

                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);

                if (_xxyy[2] == EMPTY)/*sub2 X X 1 * * 0 * 0' */
                    Chess_type[SLEEP3]++;
                else if (_xxyy[2] == flag)/*sub2 X X 1 * * 0 * *' */ {
                    Chess_type[L_RUSH4]++;
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                }
                else/*sub2 X X 1 * * 0 * 1' */
                    Chess_type[NOTHREAT]++;
            }
            else
                /*sub X X 1 * * 0 1' X */
                Chess_type[NOTHREAT]++;
        }

        /*head X X 0' * * 1' X X */
        else if (_xxyy[0] == 3 - flag) {//����һ��û�б���
            if (_xy[1] == EMPTY) {
                /*sub X 0' 0 * * 1 X X */

                if (_xy[2] == EMPTY)/*sub2 0' 0 0 * * 1 X X */
                    Chess_type[SLEEP2]++;
                else if (_xy[2] == flag) {/*sub2 *' 0 0 * * 1 X X */
                    Chess_type[SLEEP3]++;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                }
                else/*sub2 1' 0 0 * * 1 X X */
                    Chess_type[NOTHREAT]++;
            }
            else if (_xy[1] == flag) {
                /*sub X *' 0 * * 1 X X */

                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);

                if (_xy[2] == EMPTY)/*sub2 0' * 0 * * 1 X X */
                    Chess_type[SLEEP3]++;
                else if (_xy[2] == flag)/*sub2 *' * 0 * * 1 X X */ {
                    Chess_type[L_RUSH4]++;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                }
                else/*sub2 1' * 0 * * 1 X X */
                    Chess_type[NOTHREAT]++;
            }
            else
                /*sub X 1' 0 * * 1 X X */
                Chess_type[NOTHREAT]++;
        }
        break;
    case 1://һ������(�������������ǰ���ֻ�����ֻ�ж��������������)

        /*head X X X 0 * 0 X X X */
        if (_xy[0] == EMPTY && _xxyy[0] == EMPTY) {

            if ((_xy[1] == 3 - flag && _xxyy[1] == 3 - flag) ||
                (_xy[1] == 3 - flag && _xxyy[2] == 3 - flag) ||
                (_xy[2] == 3 - flag && _xxyy[1] == 3 - flag))
                /*sub X X 1 0 * 0 1 X X */
                /*sub X X 1 0 * 0 X 1 X */
                /*sub X 1 X 0 * 0 1 X X */
                Chess_type[NOTHREAT]++;


            else if (_xy[1] == EMPTY && _xxyy[1] == EMPTY) {
                /*sub X X 0 0 * 0 0 X X */

                if (_xy[2] == flag && _xy[3] == EMPTY) {/*sub2 0 * 0 0 * 0 0 X X */
                    Chess_type[B_JUMP2]++;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                }
                else if (_xy[2] == flag && _xy[3] == flag) {/*sub2 * * 0 0 * 0 0 X X */
                    Chess_type[SLEEP3]++;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                    mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
                }
                else if (_xy[2] == flag && _xy[3] == 3 - flag) {/*sub2 1 * 0 0 * 0 0 X X */
                    Chess_type[SLEEP2]++;
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                }

                if (_xxyy[2] == flag && _xxyy[3] == EMPTY) {/*sub2 X X 0 0 * 0 0 * 0 */
                    Chess_type[B_JUMP2]++;
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                }
                else if (_xxyy[2] == flag && _xxyy[3] == flag) {/*sub2 X X 0 0 * 0 0 * * */
                    Chess_type[SLEEP3]++;
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                    mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);
                }
                else if (_xxyy[2] == flag && _xxyy[3] == 3 - flag) {/*sub2 X X 0 0 * 0 0 * 1 */
                    Chess_type[SLEEP2]++;
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                }
            }

            else if (_xy[1] == flag && _xxyy[1] == flag) {
                /*sub X X * 0 * 0 * X X */
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);

                if (_xy[2] == EMPTY && _xxyy[2] == EMPTY)/*sub2 X 0 * 0 * 0 * 0 X */
                    Chess_type[SLEEP3]++;
                else if (_xy[2] == flag && _xxyy[2] == flag)/*sub2 X * * 0 * 0 * * X */ {
                    mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                    mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);

                    if (_xy[3] == 3 - flag && _xxyy[3] == 3 - flag)/*sub2 1 * * 0 * 0 * * 1 */
                        Chess_type[SLEEP3]++;
                    else if (_xy[3] == flag && _xxyy[3] == flag)/*sub2 * * * 0 * 0 * * * */ {
                        mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
                        mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);
                        Chess_type[ALIVE4]++;
                    }
                    else if (_xy[3] == flag && _xy[4] == 3 - flag)/*sub2 1 * * * 0 * 0 * * 1 */ {
                        mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
                        Chess_type[RUSH4]++;
                    }
                    else if (_xy[3] == flag && _xy[4] == EMPTY)/*sub2 0 * * * 0 * 0 * * 1 */ {
                        mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
                        Chess_type[ALIVE4]++;
                    }
                    else if (_xxyy[3] == flag && _xxyy[4] == 3 - flag)/*sub2 1 * * 0 * 0 * * * 1 */ {
                        mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);
                        Chess_type[RUSH4]++;
                    }
                    else if (_xxyy[3] == flag && _xxyy[4] == EMPTY)/*sub2 1 * * 0 * 0 * * * 0 */ {
                        mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);
                        Chess_type[ALIVE4]++;
                    }
                    else/*sub2 0' * * 0 * 0 * * 0' */
                        Chess_type[ALIVE3]++;
                }

                else if (_xy[2] == 3 - flag && _xxyy[2] == 3 - flag)/*sub2 X 1 * 0 * 0 * 1 X */
                    Chess_type[SLEEP3]++;

                else if (_xy[2] == 3 - flag)/*sub2 X 1 * 0 * 0 * X X */ {
                    if (_xxyy[2] == flag && _xxyy[3] == EMPTY) {/*sub2 X 1 * 0 * 0 * * 0 */
                        mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                        Chess_type[ALIVE3]++;
                    }
                    else if (_xxyy[2] == flag && _xxyy[3] == flag) {/*sub2 X 1 * 0 * 0 * * * */
                        mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);
                        mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);
                        if (_xxyy[4] == EMPTY)/*sub3 X 1 * 0 * 0 * * * 0 */
                            Chess_type[ALIVE4]++;
                        else if (_xxyy[4] == 3 - flag)/*sub3 X 1 * 0 * 0 * * * 1 */
                            Chess_type[L_RUSH4]++;
                    }
                    else if (_xxyy[2] == EMPTY)/*sub2 X 1 * 0 * 0 * 0 X */
                        Chess_type[SLEEP3]++;
                }
                else if (_xxyy[2] == 3 - flag)/*sub2 X X * 0 * 0 * 1 X */ {//����
                    if (_xy[2] == flag && _xy[3] == EMPTY) {
                        mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                        Chess_type[ALIVE3]++;
                    }
                    else if (_xy[2] == flag && _xy[3] == flag) {
                        mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);
                        mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
                        if (_xy[4] == EMPTY)
                            Chess_type[ALIVE4]++;
                        else if (_xy[4] == 3 - flag)
                            Chess_type[L_RUSH4]++;
                    }
                    else if (_xy[2] == EMPTY)
                        Chess_type[SLEEP3]++;
                }


            }
            else if (_xy[1] == flag && _xy[2] == EMPTY) {
                /*sub X 0 * 0 * 0 X X X */
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);

                if (_xy[3] == flag && _xy[4] == EMPTY) {/*sub2 0 * 0 * 0 * 0 X X X */
                    mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
                    Chess_type[ALIVE3]++;
                }
                else if (_xy[3] == flag && _xy[4] == 3 - flag) {/*sub2 1 * 0 * 0 * 0 X X X */
                    mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
                    Chess_type[SLEEP3]++;
                }
                else
                    Chess_type[JUMP2]++;
            }
            else if (_xxyy[1] == flag && _xxyy[2] == EMPTY) {
                /*sub X X X 0 * 0 * 0 X */
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);

                if (_xxyy[3] == flag) {/*sub2 X X X 0 * 0 * 0 * X */
                    mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);
                    Chess_type[SLEEP3]++;
                }
                else
                    Chess_type[JUMP2]++;
            }

            else if (_xy[1] == flag && _xy[2] == flag) {
                /*sub X * * 0 * 0 X X X */
                mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);
                mark_coord({ x + 2 * dir_j[dir],y + 2 * dir_i[dir] }, dir);

                if (_xy[3] == flag) {/*sub * * * 0 * 0 X X X */
                    mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);

                    if (_xy[4] == EMPTY)
                        Chess_type[RUSH4]++;
                    else if (_xy[4] == 3 - flag)
                        Chess_type[L_RUSH4]++;
                }
                else if (_xy[3] == 3 - flag)
                    Chess_type[SLEEP3]++;
                else if (_xy[3] == EMPTY)
                    Chess_type[ALIVE3]++;
            }
            else if (_xxyy[1] == flag && _xxyy[2] == flag) {
                /*���� */
                mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);
                mark_coord({ xx + 2 * dir_j[dir + 1],yy + 2 * dir_i[dir + 1] }, dir);

                if (_xxyy[3] == flag) {
                    mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);

                    if (_xxyy[4] == EMPTY)
                        Chess_type[RUSH4]++;
                    else if (_xxyy[4] == 3 - flag)
                        Chess_type[L_RUSH4]++;
                }
                else if (_xxyy[3] == 3 - flag)
                    Chess_type[SLEEP3]++;
                else if (_xxyy[3] == EMPTY)
                    Chess_type[ALIVE3]++;
            }

        }

        /*head X X X 1 * 0 * 0 * */
        else if (_xy[0] == 3 - flag && _xxyy[0] == EMPTY && _xxyy[1] == flag && _xxyy[2] == EMPTY && _xxyy[3] == flag) {
            mark_coord({ xx + 1 * dir_j[dir + 1],yy + 1 * dir_i[dir + 1] }, dir);
            mark_coord({ xx + 3 * dir_j[dir + 1],yy + 3 * dir_i[dir + 1] }, dir);
            Chess_type[SLEEP3]++;
        }
        /*head * 0 * 0 * 1 X X X */
        else if (_xy[3] == flag && _xy[2] == EMPTY && _xy[1] == flag && _xy[0] == EMPTY && _xxyy[0] == 3 - flag) {
            mark_coord({ x + 1 * dir_j[dir],y + 1 * dir_i[dir] }, dir);
            mark_coord({ x + 3 * dir_j[dir],y + 3 * dir_i[dir] }, dir);
            Chess_type[SLEEP3]++;
        }
    default:
        ;
    }
}

void Situation::evaluate_all(int flag) {//flagΪû�б���ǵ�flag
    for (int dir = U2D; dir < 8; dir += 2) {//����������ж�
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == flag + (dir / 2) * TEMP)
                    evaluate_type({ i,j }, dir);
            }
        }
    }
    mark_clear();
}

bool space_judge(Command coord) {//������true��˵����λ��̫ƫƧ
    for (int x = coord.x - 2; x <= coord.x + 2; x++) {
        if (x < 0 || x >= BOARD_SIZE)
            continue;
        for (int y = coord.y - 2; y <= coord.y + 2; y++) {
            if (y < 0 || y >= BOARD_SIZE)
                continue;
            if (board[x][y] != 0)
                return false;
        }
    }
    return true;
}

bool is_five(Coord coord) {
    int flag;
    if (board[coord.x][coord.y] == EMPTY)
        return false;
    else
        flag = board[coord.x][coord.y];
    for (int dir = U2D; dir < 8; dir += 2) {

        int count = 1;

        int x, xx, y, yy;//x��y������ֱ���ϵĵ�һ�������ϱ���������xx��yyΪ�ڶ��������ϵģ�����U2D��һ��ڶ�����ֱ����UP��DOWN
        x = coord.x + dir_j[dir], y = coord.y + dir_i[dir];
        xx = coord.x + dir_j[dir + 1], yy = coord.y + dir_i[dir + 1];

        for (; is_in_boundary({ x,y }) && flagcmp(board[x][y], flag); x += dir_j[dir], y += dir_i[dir]) {
            count++;
        }
        for (; is_in_boundary({ xx,yy }) && flagcmp(board[xx][yy], flag); xx += dir_j[dir + 1], yy += dir_i[dir + 1]) {
            count++;
        }
        if (count >= 5)
            return 1;
    }
    return 0;
}

class Decis_Tree {
public:
    Decis_Tree() {}
    int creat_tree(Node& node, int depth);
    int cal_leave(int i, int j, Node& node, int& our_value, int& enemy_value, int& temp_value, int& child_exvalue);
    int cal_branch(int& i, int& j, Node& node, int& our_value, int& child_exvalue, int& situ_count, int& depth);
    void delete_tree(Node& node);
};

int Decis_Tree::cal_leave(int i,int j,Node& node,int& our_value,int& enemy_value,int& temp_value,int& child_exvalue) {
    if (board[i][j] == EMPTY && !space_judge({ i,j }))
    {
        board[i][j] = node.flag;//��ʱ�ı�����״̬

        if (is_five({ i,j })) {//��ɽ��������������������
            if (node.flag == myFlag)
                node.value = L1 * 2;
            else
                node.value = -L1 * 2;
            node.desi = { i,j };
            for (; !node.children.empty();) {
                delete_tree(node.children.back());
                node.children.pop_back();
            }
            board[i][j] = EMPTY;//���������̵ĸı�
            return 0;
        }


        Situation enemy, us;

        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
        our_value = us.cal_value(0);
        enemy.evaluate_all(enemyFlag);
        enemy_value = enemy.cal_value(1);

        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

        if (node.max_min == 1) {//max���

            if (temp_value > node.ex_value) {//��֦
                board[i][j] = EMPTY;//���������̵ĸı�
                delete_tree(node);
                return -1;
            }
            if (temp_value > child_exvalue) {
                child_exvalue = temp_value;
                node.desi = { i,j };
            }
        }
        else {//min���

            if (temp_value < node.ex_value) {//��֦
                board[i][j] = EMPTY;//���������̵ĸı�
                delete_tree(node);
                return -1;
            }
            if (temp_value < child_exvalue) {
                child_exvalue = temp_value;
                node.desi = { i,j };
            }
        }
        board[i][j] = EMPTY;//���������̵ĸı�
    }
    return 100;
}

int Decis_Tree::cal_branch(int& i, int& j, Node& node, int& temp_value, int& child_exvalue, int& situ_count, int& depth) {
    if (board[i][j] == EMPTY && !space_judge({ i,j }))
    {
        board[i][j] = node.flag;//��ʱ�ı�����״̬
        situ_count++;

        if (is_five({ i,j })) {//��ɽ��������������������
            if (node.flag == myFlag)
                node.value = L1 * 2;
            else
                node.value = -L1 * 2;
            node.desi = { i,j };
            for (; !node.children.empty();) {
                delete_tree(node.children.back());
                node.children.pop_back();
            }
            board[i][j] = EMPTY;//���������̵ĸı�
            return 0;
        }

        Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
        child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

        if (creat_tree(child, depth - 1) != -1) {
            node.children.push_back(child);//��������
            temp_value = child.value;

            if (node.max_min == 1) {//max���
                if (temp_value > node.ex_value) {
                    board[i][j] = EMPTY;//���������̵ĸı�
                    delete_tree(node);
                    return -1;
                }
                if (temp_value >= child_exvalue) {
                    int i = node.children.back().x, j = node.children.back().y;
                    if (temp_value == child_exvalue) {
                        Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                            re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                        locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                        locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                        re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                        re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                        if (Round <= REAR) {
                            if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                node.desi = { i,j };
                        }
                        else {
                            if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                node.desi = { i,j };
                        }
                    }
                    else {
                        child_exvalue = temp_value;
                        node.desi = { i,j };
                    }
                }
            }
            else {//min���
                if (temp_value < node.ex_value) {
                    board[i][j] = EMPTY;//���������̵ĸı�
                    delete_tree(node);
                    return -1;
                }
                if (temp_value <= child_exvalue) {
                    int i = node.children.back().x, j = node.children.back().y;
                    if (temp_value == child_exvalue) {
                        Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                            re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                        locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                        locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                        re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                        re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                        if (Round <= REAR) {
                            if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                node.desi = { i,j };
                        }
                        else {
                            if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                node.desi = { i,j };
                        }
                    }
                    else {
                        child_exvalue = temp_value;
                        node.desi = { i,j };
                    }
                }
            }
        }

        board[i][j] = EMPTY;//���������̵ĸı�
    }


    return 100;
}

//�׸�����Ľ��ֻ�ñ�עmax/min��ڰ��������һ���µ������ɫ
int Decis_Tree::creat_tree(Node& node, int depth) {//node��flag������һ��Ҫ�ߵ���

    if (depth - 1 <= 0) {//��ָ����������ֹ
        int temp_value = 0;//�ӽڵ���������value
        int enemy_value = 0, our_value = 0;//����˫��������ֵ
        int child_exvalue = 0;//�ӽڵ�value�ļ�ֵ

        if (node.max_min == 1)
            child_exvalue = INT_MIN;
        else
            child_exvalue = INT_MAX;
        int i = 0, j = 0;//��¼��ǰ����
        int y = node.x, x = node.y;//������������
        int maxi = y > (BOARD_SIZE - 1 - y) ? y : (BOARD_SIZE - 1 - y);
        int maxj = x > (BOARD_SIZE - 1 - x) ? x : (BOARD_SIZE - 1 - x);
        int max_lap = maxi > maxj ? maxi : maxj;

        //���Ƕ��forѭ����Ŀ���ǽ�������ʽ����������ѭ���е�ѭ���嶼��ͬ
        for (int lap = 1; lap < max_lap + 1; lap++) {
            int ii = y + lap;//��
            if (ii < BOARD_SIZE) {
                for (int n = 0; n <= lap; n++) {
                    i = ii, j = x + n;
                    if (j >= BOARD_SIZE)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
                for (int n = 1; n <= lap; n++) {
                    i = ii, j = x - n;
                    if (j < 0)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
            }
            ii = y - lap;//��
            if (ii >= 0) {
                for (int n = 0; n <= lap; n++) {
                    i = ii, j = x + n;
                    if (j >= BOARD_SIZE)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
                for (int n = 1; n <= lap; n++) {
                    i = ii, j = x - n;
                    if (j < 0)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
            }
            int jj = x + lap;//��
            if (jj < BOARD_SIZE) {
                for (int n = 0; n < lap; n++) {
                    i = y + n, j = jj;
                    if (i >= BOARD_SIZE)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
                for (int n = 1; n < lap; n++) {
                    i = y - n, j = jj;
                    if (i < 0)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
            }
            jj = x - lap;//��
            if (jj >= 0) {
                for (int n = 0; n < lap; n++) {
                    i = y + n, j = jj;
                    if (i >= BOARD_SIZE)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
                for (int n = 1; n < lap; n++) {
                    i = y - n, j = jj;
                    if (i < 0)
                        break;
                    //����Ҷ���
                    if (board[i][j] == EMPTY && !space_judge({ i,j }))
                    {
                        board[i][j] = node.flag;//��ʱ�ı�����״̬

                        if (is_five({ i,j })) {//��ɽ��������������������
                            if (node.flag == myFlag)
                                node.value = L1 * 2;
                            else
                                node.value = -L1 * 2;
                            node.desi = { i,j };
                            for (; !node.children.empty();) {
                                delete_tree(node.children.back());
                                node.children.pop_back();
                            }
                            board[i][j] = EMPTY;//���������̵ĸı�
                            return 0;
                        }


                        Situation enemy, us;

                        us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                        our_value = us.cal_value(0);
                        enemy.evaluate_all(enemyFlag);
                        enemy_value = enemy.cal_value(1);

                        temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                        if (node.max_min == 1) {//max���

                            if (temp_value > node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value > child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        else {//min���

                            if (temp_value < node.ex_value) {//��֦
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value < child_exvalue) {
                                child_exvalue = temp_value;
                                node.desi = { i,j };
                            }
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                    }
                }
            }
        }

        /*for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                cal_leave(i, j, node, our_value, enemy_value, temp_value, child_exvalue);
                /*if (board[i][j] == EMPTY && !space_judge({i,j}))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }


                    Situation enemy, us;

                    us.evaluate_all(myFlag);//���㼺����з��Ĺ�ֵ
                    our_value = us.cal_value(0);
                    enemy.evaluate_all(enemyFlag);
                    enemy_value = enemy.cal_value(1);

                    temp_value = our_value - enemy_value * DEFENCE_WEIGHT / ATTACK_WEIGHT;

                    if (node.max_min == 1) {//max���

                        if (temp_value > node.ex_value) {//��֦
                            board[i][j] = EMPTY;//���������̵ĸı�
                            delete_tree(node);
                            return -1;
                        }
                        if (temp_value > child_exvalue) {
                            child_exvalue = temp_value;
                            node.desi = { i,j };
                        }
                    }
                    else {//min���

                        if (temp_value < node.ex_value) {//��֦
                            board[i][j] = EMPTY;//���������̵ĸı�
                            delete_tree(node);
                            return -1;
                        }
                        if (temp_value < child_exvalue) {
                            child_exvalue = temp_value;
                            node.desi = { i,j };
                        }
                    }
                    board[i][j] = EMPTY;//���������̵ĸı�
                }*/
           // }
        //}

        node.value = child_exvalue;
        return 0;
    }//ָ��������ֹ

    int temp_value = 0;//�ӽڵ���������value
    int child_exvalue = 0;//�ӽڵ�value�ļ�ֵ

    int situ_count = 0;

    if (node.max_min == 1)
        child_exvalue = INT_MIN;
    else
        child_exvalue = INT_MAX;

    int i = 0, j = 0;//��¼��ǰ����
    int y = node.x, x = node.y;//������������
    if (depth == MAX_DEPTH)
        y = pi, x = pj;
    int maxi = y > (BOARD_SIZE - 1 - y) ? y : (BOARD_SIZE - 1 - y);
    int maxj = x > (BOARD_SIZE - 1 - x) ? x : (BOARD_SIZE - 1 - x);
    int max_lap = maxi > maxj ? maxi : maxj;
    for (int lap = 1; lap < max_lap + 1; lap++) {
        int ii = y + lap;//��
        if (ii < BOARD_SIZE) {
            for (int n = 0; n <= lap; n++) {
                i = ii, j = x + n;
                if (j >= BOARD_SIZE)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
            for (int n = 1; n <= lap; n++) {
                i = ii, j = x - n;
                if (j < 0)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
        }
        ii = y - lap;//��
        if (ii >= 0) {
            for (int n = 0; n <= lap; n++) {
                i = ii, j = x + n;
                if (j >= BOARD_SIZE)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
            for (int n = 1; n <= lap; n++) {
                i = ii, j = x - n;
                if (j < 0)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
        }
        int jj = x + lap;//��
        if (jj < BOARD_SIZE) {
            for (int n = 0; n < lap; n++) {
                i = y + n, j = jj;
                if (i >= BOARD_SIZE)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
            for (int n = 1; n < lap; n++) {
                i = y - n, j = jj;
                if (i < 0)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
        }
        jj = x - lap;//��
        if (jj >= 0) {
            for (int n = 0; n < lap; n++) {
                i = y + n, j = jj;
                if (i >= BOARD_SIZE)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
            for (int n = 1; n < lap; n++) {
                i = y - n, j = jj;
                if (i < 0)
                    break;
                //�����֧�ڵ�
                if (board[i][j] == EMPTY && !space_judge({ i,j }))
                {
                    board[i][j] = node.flag;//��ʱ�ı�����״̬
                    situ_count++;

                    if (is_five({ i,j })) {//��ɽ��������������������
                        if (node.flag == myFlag)
                            node.value = L1 * 2;
                        else
                            node.value = -L1 * 2;
                        node.desi = { i,j };
                        for (; !node.children.empty();) {
                            delete_tree(node.children.back());
                            node.children.pop_back();
                        }
                        board[i][j] = EMPTY;//���������̵ĸı�
                        return 0;
                    }

                    Node child(i, j, 3 - node.flag, 1 - node.max_min);//�������
                    child.ex_value = child_exvalue;//������һ����ļ�ֵ�Ա��ڼ�֦

                    if (creat_tree(child, depth - 1) != -1) {
                        node.children.push_back(child);//��������
                        temp_value = child.value;

                        if (node.max_min == 1) {//max���
                            if (temp_value > node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value >= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2 ,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;
                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ�ȣ�ѡ����
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��,ѡԶ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                        else {//min���
                            if (temp_value < node.ex_value) {
                                board[i][j] = EMPTY;//���������̵ĸı�
                                delete_tree(node);
                                return -1;
                            }
                            if (temp_value <= child_exvalue) {
                                int i = node.children.back().x, j = node.children.back().y;
                                if (temp_value == child_exvalue) {
                                    Coord locate = { i - BOARD_SIZE / 2 ,j - BOARD_SIZE / 2 },
                                        re_locate = { node.desi.x - BOARD_SIZE / 2,node.desi.y - BOARD_SIZE / 2 };
                                    locate.x > 0 ? locate.x = locate.x : locate.x = -locate.x;
                                    locate.y > 0 ? locate.y = locate.y : locate.y = -locate.y;
                                    re_locate.x > 0 ? re_locate.x = re_locate.x : re_locate.x = -re_locate.x;
                                    re_locate.y > 0 ? re_locate.y = re_locate.y : re_locate.y = -re_locate.y;

                                    if (Round <= REAR) {
                                        if (locate.x + locate.y < re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                    else {
                                        if (locate.x + locate.y > re_locate.x + re_locate.y)//�ж�λ�þ������ĵ�ƫƧ��
                                            node.desi = { i,j };
                                    }
                                }
                                else {
                                    child_exvalue = temp_value;
                                    node.desi = { i,j };
                                }
                            }
                        }
                    }

                    board[i][j] = EMPTY;//���������̵ĸı�
                }
            }
        }
    }

    node.value = child_exvalue;
    if (situ_count == 0)
        return -1;
    return 0;
}

void Decis_Tree::delete_tree(Node& node) {
    for (; !node.children.empty();) {
        delete_tree(node.children.back());
        node.children.pop_back();
    }
    node.children.clear();
}

void debug(const char* str)
{
    printf("DEBUG %s\n", str);
    fflush(stdout);
}
BOOL isInBound(int x, int y)
{
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}
void place(struct Command cmd)
{
    board[cmd.x][cmd.y] = enemyFlag;
}
void initAI(int me)
{
    enemyFlag = 3 - me;
}
void start(int flag)
{
    memset(board, 0, sizeof(board));//��ʼ������
    int middlePlace = BOARD_SIZE / 2;
    board[middlePlace - 1][middlePlace - 1] = WHITE;
    board[middlePlace][middlePlace] = WHITE;
    board[middlePlace - 1][middlePlace] = BLACK;
    board[middlePlace][middlePlace - 1] = BLACK;
    initAI(flag);
}

void turn(int& ii, int& jj)
{
    Node root(-1, -1, myFlag, 1);
    Decis_Tree T;
    int depth = MAX_DEPTH;
    if (T.creat_tree(root, depth) == -1) {
        T.delete_tree(root);
        return;
    }
    int i = root.desi.x, j = root.desi.y;
    T.delete_tree(root);
    printf("%d %d", i, j);
    printf("\n");
    fflush(stdout);
    board[i][j] = myFlag;
    ii = i, jj = j;
    return;
}
void end(int x)
{
    exit(0);
}

void loop()
{
    char tag[10] = { 0 };
    struct Command command =
    {
        0,
        0
    };
    if (myFlag == WHITE)
        DEFENCE_WEIGHT = 20;
    int status;
    while (TRUE)
    {
        memset(tag, 0, sizeof(tag));
        scanf("%s", tag);
        if (strcmp(tag, START) == 0)
        {
            scanf("%d", &myFlag);
            start(myFlag);
            printf("OK\n");
            fflush(stdout);
        }
        else if (strcmp(tag, PLACE) == 0)
        {
            scanf("%d %d", &command.x, &command.y);
            place(command);
        }
        else if (strcmp(tag, TURN) == 0)
        {
            int i, j;
            Round++;
            if (Round == 10)
                DEFENCE_WEIGHT = 15;
            if (Round == 1 && myFlag == BLACK) {
                i = 7, j = 6;
                printf("%d %d", i, j);
                printf("\n");
                fflush(stdout);
                board[i][j] = myFlag;
            }
            else
                turn(i, j);
        }
        else if (strcmp(tag, END) == 0)
        {
            scanf("%d", &status);
            end(status);
        }
    }
}

bool is_draw() {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] == EMPTY)
                return 0;
    return 1;
}

void print_board() {
    system("cls");
    printf("    ");
    for (int i = 0; i < BOARD_SIZE; i++)
        printf("%3dC", i);
    printf("\n");
    printf("   ");
    printf("��");
    for (int j = 0; j < BOARD_SIZE; j++) {
        printf("--�� ");
    }
    printf("-�� ");
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2dR��-", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            switch (board[i][j]) {
            case EMPTY:
                printf("-��-");
                break;
            case BLACK:
                printf("-��-");
                break;
            case WHITE:
                printf("-��-");
                break;
            default:
                ;
            }
        }
        printf("��\n");
        printf("   �� ");
        if (i != BOARD_SIZE)
            for (int j = 0; j < BOARD_SIZE; j++) {
                printf(" �� ");
            }
        printf("��");
        printf("\n");
    }
    printf("   ��");
    for (int j = 0; j < BOARD_SIZE; j++) {
        printf("--�� ");
    }
    printf("-�� ");
    printf("\n");

}
void print_board0() {
    system("cls");
    printf("    ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (i < 10)
            printf("%2d", i);
        else
            printf("%2c", i - 10 + 'A');
    }
    printf("\n");
    printf("   ");
    printf("��");
    for (int j = 0; j < BOARD_SIZE; j++) {
        printf("�� ");
    }
    printf("�� ");
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2dR��", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            switch (board[i][j]) {
            case EMPTY:
                printf("��");
                break;
            case BLACK:
                printf("��");
                break;
            case WHITE:
                printf("��");
                break;
            default:
                ;
            }
        }
        printf("��\n");
    }
    printf("   ��");
    for (int j = 0; j < BOARD_SIZE; j++) {
        printf("�� ");
    }
    printf("�� ");
    printf("\n");

}

void loop_print() {
    char tag[10] = { 0 };
    struct Command command =
    {
        0,
        0
    };
    if (myFlag == WHITE)
        DEFENCE_WEIGHT = 100;
    int status;
    while (TRUE)
    {
        printf("����START��ʼ��Ϸ:\n");
        memset(tag, 0, sizeof(tag));
        scanf("%s", tag);
        if (strcmp(tag, START) == 0)
        {
            printf("��ѡ��ڷ��׷�(1Ϊ�ף�2Ϊ��)��\n");
            scanf("%d", &myFlag);
            start(myFlag);
            printf("OK\n");
            fflush(stdout);
            break;
        }
    }
    print_board();
    system("pause");
    int current_flag = 0;//0�������巽��1����AI
    if (myFlag == BLACK)
        current_flag = 1;
    while (TRUE)
    {
        if (is_draw()) {
            print_board();
            printf("Draw!\n");
            system("pause");
            break;
        }

        if (current_flag == 0)
        {
            //print_board();

            printf("�����뼺����������(���к���,�ո����,��'7 7'):");
            scanf("%d %d", &command.x, &command.y);
            if (board[command.x][command.y] != EMPTY) {
                printf("����Ƿ�������������\n");
                system("pause");
                continue;
            }
            place(command);
            print_board();
            printf("��������:%dR %dC\n", command.x, command.y);
            if (is_five({ command.x,command.y })) {
                print_board();
                printf("You Win!\n");
                system("pause");
                break;
            }
            //system("pause");
        }
        else if (current_flag == 1)
        {
            int i, j;
            Round++;
            if (Round == 10)
                DEFENCE_WEIGHT = 15;

            //��������
            if (Round == 1 && myFlag == BLACK) {
                i = 7, j = 6;
                printf("%d %d", i, j);
                printf("\n");
                fflush(stdout);
                board[i][j] = myFlag;
            }
            else
                turn(i, j);

            print_board();
            printf("�з�����:%dR %dC\n", i, j);
            if (is_five({ i,j })) {
                print_board();
                printf("You Lose!\n");
                system("pause");
                break;
            }
            //system("pause");
        }

        current_flag = 1 - current_flag;
    }
}

int main(int argc, char* argv[])
{
    loop_print();
    return 0;
}