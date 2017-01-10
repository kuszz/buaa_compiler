#include <iostream>


#define ident				1
#define	integer				2
#define character	        3
#define	text				4
#define eof                 5

#define 	CONST			10
#define	    INT 			11
#define     CHAR            12
#define	    VOID			13
#define     MAIN			14
#define	    IF			    15
#define	    ELSE			16
#define	    DO			    17
#define	    WHILE			18
#define	    FOR  			19
#define	    SCANF			20
#define	    PRINTF			21
#define	    RETURN			22


#define	PLUS				31
#define	MINUS				32
#define	MUL				    33
#define	DIV  				34
#define	R_BIG			    35
#define	L_BIG			    36
#define R_MIDDLE            37
#define L_MIDDLE            38
#define	R_SMALL			    39
#define	L_SMALL			    40
#define	SEMICOLON			41				// ;
#define	COMMA				42				//,
#define	D_QUOTA          	43
#define	QUOTA    			44
#define	GREATER				45
#define	LESS				46
#define D_EQUAL				47
#define	NOT_EQUAL			48
#define	G_EQUAL		        49
#define	L_EQUAL     		50
#define	EQUAL 				51


#define inttype   101
#define chartype  102
#define constinttype 103
#define constchartype 104
#define functype 105
#define paratype 106



typedef struct{
 char name[30];
 int  type;
 int  value; //�����constint����constchar�Ļ�����ֵ�� ���kindΪ����kindʱ��1�����з���ֵ������0�����޷���ֵ����
 int  size; //���������Ļ����洢�����size��С�������Ļ�1������ֵΪchar��0����int,-1����void�������sizeΪ0��
 int  paranum;//����Ǻ����Ļ����в�������������ʱ��Ϊ0��
 int  level;
}table;

typedef struct{
 char op[10];
 char var1[200];
 char var2[30];
 char var3[30];
}quaternary;

typedef struct _varNode{
	char id[20];//����
	int add;//��Ի�ַ��ƫ��������ʵ���Ǳ�������������i��
	int type;//1 char 0 int
	struct _varNode *next;
}varNode;

typedef struct{
	quaternary code[1000];
	int codeNum;
	int nextBlock[2];
	char useVar[50][50];
	int useNum;
	char defVar[50][50];
	int defNum;
	char inVar[50][50];
	int inNum;
	char outVar[50][50];
	int outNum;
}block;

typedef struct{
	int block[500];
	int blockNum;
}flowDiagram;

typedef struct{
	int isExported;
	int nodeNum;//����ڵ��
	int leftNum;//���ӽڵ��
	int rightNum;//�Һ��ӽڵ��
	char labelName[100];//�ڵ�������
	int label;//0Ҷ�ڵ㣬1�м�ڵ�
}dagNode;

typedef struct{
	int nodeNumber[100];
	int counter;
	char nodeName[100];
}tabNode;

typedef struct{
	char varName[50];
	char conflictVar[50][50];
	int conNum;
	int Reg;//0��ʾ����ESI��1��ʾ����EDI
	int tmpNum;////�ڷ���ʱ����ĳ���ڵ��ʣ�³�ͻ����Ŀ
	int isAllocted;
}Conflict;
