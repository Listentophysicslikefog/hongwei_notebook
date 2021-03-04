/*  点库接口“point.h”*/

struct POINT;
typedef struct POINT * PPOINT;  //这里只可以定义指针，不可以定义对象，因为结构体还没有定义

PPOINT PtCreate( int x, int y );
void PtDestroy( PPOINT point );
void PtGetValue( POINT point, int * x, int * y );
void PtSetValue( PPOINT point, int x, int y );
bool PtCompare( PPOINT point1, PPOINT point2 );
char * PtTransformIntoString( PPOINT point );
void PtPrint( PPOINT point );