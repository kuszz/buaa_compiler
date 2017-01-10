#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "define.h"

using namespace std;

int funcTab[200]={0};//�ֳ��򣨺�������������
int funcIndex =0;//�ֳ�������ָ��i�����ʱָ����ǵ�ǰ�����ķֳ������������е�λ�ã�����ǰ�����ڷ��ű��е���ʼλ�ã�
int tabIndex;//���ű�ָ�룬ÿ����1���ű�����1,ʵ��ջ��ΪtabIndex-1
int paraNums=0,type=-1,value=-1,size,level;
int tempSym;
int hasReturn=0,hasMain=0;
int labelNum=0;
int funcTmpNum=0;
int codeNum=0;
int printChar;
FILE* fp;
char proName[30];
char midValue[30];
table Table[200];
quaternary code[1000];

char   id[30] ;
extern int   symbol;
extern int   number;
extern int   err;
extern int   lineNum;
extern string errorde[30];
extern char token[82];
extern char ch;
extern FILE* fin;

extern void getsym();
extern void error(int e);

void statement();
void stateList();
void valParameter();
void expression();

char* createLabel(){
	char *newLabel = (char *)malloc(10*sizeof(char));
	char counter[10];
	memset(newLabel,0,10*sizeof(char));
	newLabel[0] = 'l';
	newLabel[1] = 'a';
	newLabel[2] = 'b';
	strcat(newLabel,itoa(labelNum++,counter,10));
	return newLabel;
}

char* tempVal(){
	char *newVal = (char *)malloc(10*sizeof(char));
	char counter[10];
	memset(newVal,0,10*sizeof(char));
	newVal[0] = '$';
	itoa(funcTmpNum++,counter,10);
	strcat(newVal,counter);
	return newVal;
}

void emit(char op[],char v1[],char v2[],char v3[]){
	strcpy(code[codeNum].op,op);
	strcpy(code[codeNum].var1,v1);
	strcpy(code[codeNum].var2,v2);
	strcpy(code[codeNum++].var3,v3);
}

bool judgeEOF(){
    if (symbol==eof) return true;
    else return false;
}

void skip(int op){
    if(op==0 || op==1 || op==2){
        while (symbol!=CONST && symbol!=INT && symbol!=CHAR && symbol!=VOID && symbol!=IF
            && symbol!=DO && symbol!=FOR && symbol!=RETURN && symbol!=SCANF && symbol!=PRINTF){
            if(judgeEOF())return;
            getsym();
        }
    }else if(op==3 || op==4){
        while (symbol!=SEMICOLON) {
            if(judgeEOF())return;
            getsym();
        }
    }else if(op==5){
        while (symbol!=INT && symbol!=CHAR&& symbol!=VOID) {
            if(judgeEOF())return;
            getsym();
        }
    }
}

bool checkNextSym(int need,int sym,int e,int sk){
    if(need==1)getsym();
    if(symbol!=sym){
        error(e);
        skip(sk);
        return true;
    }else
        return false;
}

void midcodeToFile(){
	int i= 0;
    fp=fopen("midcode.txt","w+");
	while (i<codeNum) {
        fprintf(fp,"%s,\t",code[i].op);
        fprintf(fp,"%s,\t",code[i].var1);
        fprintf(fp,"%s,\t",code[i].var2);
        fprintf(fp,"%s;\n",code[i].var3);
		if (strcmp(code[i].op,"end")==0)
			fprintf(fp,"\n");
		i++;
	}
	fclose(fp);
	return;
}

void insertTable(char nam[],int typ, int val,int siz,int par,int index,int lev){
     int tmpIndex;
     if(index>199){
         error(1);
         return ;
     }
     if(typ==functype){
        tmpIndex=1;
        while(tmpIndex<=funcIndex){
            if(strcmp(Table[funcTab[tmpIndex]].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
        tmpIndex=0;
        while(tmpIndex<=funcTab[1]){
            if(strcmp(Table[tmpIndex].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
     }else {
        tmpIndex=funcTab[funcIndex];
        while(tmpIndex<index){ //�����ڵ�ǰ�����в����Ƿ����ظ�����
            if(strcmp(Table[tmpIndex].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
        tmpIndex=0;
        if(typ!=paratype)//�����������ȷ�����ű��У��ǲ����ż�������
          while(tmpIndex<funcTab[1]){//��ȫ�ֱ����в����Ƿ����ظ�����
            if(strcmp(Table[tmpIndex].name,nam)==0&&Table[tmpIndex].level==lev){
                error(5);
                return;
            }
            tmpIndex++;
        }
     }
     strcpy(Table[index].name,nam);
     Table[index].type=typ;
     Table[index].value=val;
     Table[index].size=siz;
     Table[index].paranum=par;
     Table[index].level=lev;
  if(typ==functype){
     funcTab[++funcIndex]=index;
  }
      tabIndex++;
}

void clearFuncTab(){
    int temp1 = tabIndex-1;
	int temp2;
	while((Table[temp1].type==inttype || Table[temp1].type==chartype ||Table[temp1].type==constinttype
		   || Table[temp1].type==constchartype|| Table[temp1].type==paratype)){
		Table[temp1].type = 0;
		Table[temp1].size = 0;
		Table[temp1].paranum = 0;
		Table[temp1].value = 0;
		temp2 = 0;
		while (temp2<30)
			Table[temp1].name[temp2++] = '\0';
		temp1--;
	}
	tabIndex = temp1+1;////�����µķ��ű�ջ��ָ��
	return;
}

int searchTab(char nam[],int isFunc,int isArr){
   int tmpIndex;
   if(isFunc==1){
      tmpIndex=1;
      while(tmpIndex<=funcIndex){
        if(strcmp(Table[funcTab[tmpIndex]].name,nam)==0)
            break;
        tmpIndex++;
      }
      if(tmpIndex>funcIndex){
        error(6);
        printf("%s\n",nam);
        return -1;
      }
      if (Table[funcTab[tmpIndex]].paranum!=paraNums) {
			error(9);
			return -3;
		}
      return funcTab[tmpIndex];//���ҳɹ������ڷ��ű���λ��
   }else {
         tmpIndex=funcTab[funcIndex];//�����ڵ�ǰ�ֳ�����ű��в���
         while (tmpIndex<tabIndex) {
			if (strcmp(Table[tmpIndex].name,nam)==0 && Table[tmpIndex].type!=functype && (isArr==0 || Table[tmpIndex].size>0)) return tmpIndex;
			tmpIndex++;
         }
		tmpIndex=0;
        while(tmpIndex<funcTab[1]){
            if (strcmp(Table[tmpIndex].name,nam)==0 && Table[tmpIndex].type!=functype && (isArr==0 || Table[tmpIndex].size>0)) return tmpIndex;
			tmpIndex++;
        }
        if(tmpIndex==funcTab[1]){
            error(8);
            return -1;
        }
   }
   return -1;
}

//���������壾   ::=   int����ʶ��������������{,����ʶ��������������} | char����ʶ���������ַ���{,����ʶ���������ַ���}
//��������        ::= �ۣ������ݣ��޷�������������
void constDefine(int preSym){
    if(checkNextSym(0,ident,11,3))return;
    strcpy(id,token);
    if(checkNextSym(1,EQUAL,10,3))return;
    getsym();
    if(symbol==PLUS||symbol==MINUS){
        int sign= symbol;
        if(checkNextSym(1,integer,12,3))return;
        if(preSym==INT){
            if(sign==PLUS) value = number;
            else if(sign==MINUS) value = 0-number;
        }
        insertTable(id,type,value,0,0,tabIndex,level);
        char tmpVal[30];
        itoa(value,tmpVal,10);
        emit((char*)"const",(char*)"int",tmpVal,id);
    }else {
        if(preSym==INT&&symbol==integer){
            insertTable(id,type,number,0,0,tabIndex,level);
            char tmpVal[30];
            value = number;
            itoa(value,tmpVal,10);
            emit((char*)"const",(char*)"int",tmpVal,id);
        }else if(preSym==CHAR&&symbol==character){
            value=token[0];
            insertTable(id,type,value,0,0,tabIndex,level);
            emit((char*)"const",(char*)"char",token,id);
        }else {
            error(14);
            skip(3);
            return;
        }
    }
    getsym();
    return ;
}

//������˵���� ::=  const���������壾;{ const���������壾;}
void constDeclare(){
    int preSym;
    if(checkNextSym(0,CONST,7,4))return;
    getsym();
    if (symbol==INT || symbol==CHAR) {
        if (symbol==INT) type= constinttype;
        else if (symbol==CHAR) type = constchartype;
        paraNums = 0;
        preSym=symbol;
        getsym();
        constDefine(preSym);
        while (symbol==COMMA) {
            getsym();
            constDefine(preSym);
        }
        if(checkNextSym(0,SEMICOLON,15,0))return;
        tempSym=symbol;
    }else {
        error(16);
        skip(0);
        return;
    }
	getsym();
	return;
}

//����ͷ��  int a;  char b,c; int a2[3];  int func(){}//ע����ҪԤ�����Լ���¼��ʱ���ļ�ָ��λ���Լ����أ�������
void defineHead(){
    if(symbol==INT||symbol==CHAR){
        if(symbol==INT) type=inttype;
        else type=chartype;
        if(checkNextSym(1,ident,0,3))return;
        else strcpy(id,token);
    }else {
        error(16);
        while (symbol!=COMMA && symbol!=L_SMALL && symbol!=R_SMALL) {
            if(judgeEOF())return;
            getsym();
        }
        return ;
    }
    getsym();
    return ;
}

//���������壾  ::= �����ͱ�ʶ����(����ʶ����|����ʶ������[�����޷�����������]��){,(����ʶ����|����ʶ������[�����޷�����������]�� )}
void varDefine(){
    defineHead();
    if(symbol==COMMA){
        insertTable(id,type,0,0,0,tabIndex,level);
        if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
        else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
        while (symbol==COMMA){
		    if(checkNextSym(1,ident,0,4))return;
		    strcpy(id,token);
            getsym();
		    if(symbol!=L_MIDDLE){
                insertTable(id,type,0,0,0,tabIndex,level);
                if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
            }else if(symbol==L_MIDDLE){
                if(checkNextSym(1,integer,12,4))return;
                size=number;
                if(checkNextSym(1,R_MIDDLE,18,4))return;
                insertTable(id,type,0,size,0,tabIndex,level);
                char q[30];
                itoa(size,q,10);
                if(type==inttype) emit((char*)"int",(char*)"[]",q,id);
                else if(type==chartype) emit((char*)"char",(char*)"[]",(char*)q,id);
                size=0;
                 getsym();
                 continue;
            }
        }
    }else if(symbol==L_MIDDLE){ //int a[10]
        if(checkNextSym(1,integer,12,4))return;
        size=number;
        if(checkNextSym(1,R_MIDDLE,18,4))return;
        insertTable(id,type,0,size,0,tabIndex,level);
        char q[30];
        itoa(size,q,10);
        if(type==inttype) emit((char*)"int",(char*)"[]",q,id);
        else if(type==chartype) emit((char*)"char",(char*)"[]",q,id);
        getsym();
        if(symbol==COMMA){//int a[10],b.......
            while (symbol==COMMA) {
                if(checkNextSym(1,ident,0,4))return;
		        strcpy(id,token);
                getsym();
		        if(symbol!=L_MIDDLE){
                    insertTable(id,type,0,0,0,tabIndex,level);
                    if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                    else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
                }else if(symbol==L_MIDDLE){//int a[10],b[10],......
                    if(checkNextSym(1,integer,12,4))return;
                    size=number;
                    if(checkNextSym(1,R_MIDDLE,18,4))return;
                    insertTable(id,type,0,size,0,tabIndex,level);
                    char q[30];
                    itoa(size,q,10);
                    if(type==inttype) emit((char*)"int",(char*)"[]",q,id);
                    else if(type==chartype) emit((char*)"char",(char*)"[]",(char*)"q",id);
                     getsym();
                     continue;
                }
            }
        }//int a[10],b...���������۽���
    }//int a[10] ������۽���;
    else {//�Ȳ��Ƕ���Ҳ���������ţ���ʵ����;���� (������С���Ų���������� ��ô�Ͱ������ʶ�������
        insertTable(id,type,0,0,0,tabIndex,level);
        if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
        else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
    }
    return ;
}

//������˵�����֣�  ::= ���������壾;{���������壾;}
void varDeclare(){
    varDefine();
    if(checkNextSym(0,SEMICOLON,15,0))return;
    getsym();
	return;
}

//��������    ::=  �����ͱ�ʶ��������ʶ����{,�����ͱ�ʶ��������ʶ����}| ���գ�
void parameters(){
    int i=0;     //��¼��������
	if( symbol==INT||symbol==CHAR ) {
            int tmpSym1;
            tmpSym1=symbol;
		do{
             tmpSym1=symbol;
			if (symbol==COMMA)
                getsym();
            if(tmpSym1==COMMA&&(symbol==INT||symbol==CHAR))
                tmpSym1=symbol;
			defineHead();
			type = paratype;
	    	insertTable(id,type,0,0,0,tabIndex,level);  //����������������ű�,����ҲҪ���
            if(tmpSym1==INT)
                emit((char*)"para",(char*)"int",(char*)"0",id);
            else if(tmpSym1==CHAR)
                emit((char*)"para",(char*)"char",(char*)"0",id);
    		i++;
		}while(symbol==COMMA);
	}
	paraNums = i;
	Table[funcTab[funcIndex]].paranum = paraNums;//���뺯���Ĳ�������
	return;
}

//�����ӣ� ::= ����ʶ����������ʶ������[�������ʽ����]������������|���ַ��������з���ֵ����������䣾|��(�������ʽ����)��
void factor(){
    char names[30],tmp[30];
    int res;
	if (symbol==ident) {
        strcpy(id,token);
		strcpy(names,id);
		getsym();
		if (symbol==L_SMALL) {
			getsym();
			valParameter();
			if(checkNextSym(0,R_SMALL,20,1))return;
			res = searchTab(names,1,0);
			if (res==-1 || Table[res].type!=functype ||(Table[res].type==functype&&Table[res].value==0) ) {//�Ҳ�������ı�ʶ�����߲��Ǻ������߲����з���ֵ�ĺ���
				error(8);
			     skip(1);
				 return;
			}
			if(Table[res].size==0)
                printChar++;
			strcpy(tmp,tempVal());//������ʱ����
			if(Table[res].size==0)
			emit((char*)"call",names,(char*)"int",tmp);//�����õķ���ֵ�������ʱ��������
			if(Table[res].size==1)
			emit((char*)"call",names,(char*)"char",tmp);//�����õķ���ֵ�������ʱ��������
			strcpy(midValue,tmp);
			getsym();
			return;
		}
        else if(symbol==L_MIDDLE){//����ʶ������[�������ʽ����]��
            res = searchTab(names,0,1);
            if(res<0||Table[res].size==0){////�Ҳ����������߱������Ǹ��������
                error(8);            //"="����ǲ��Ϸ��ı�ʶ��
				skip(1);
                return ;
            }
            int vl=Table[res].size;
            getsym();
            expression();
            char tmpArrVal[30];
            strcpy(tmpArrVal,midValue);
             if(tmpArrVal[0]>='0'&&tmpArrVal[0]<='9'){/////���tmpArrVal�Ǹ����֣�Ҫ�������Խ��
                int a;
                a=atoi(tmpArrVal);
                if(a>=vl){
                    error(13);
                    skip(1);
                    return ;
                }
             }
             if(checkNextSym(0,R_MIDDLE,18,1))return;
            strcpy(tmp,tempVal());
            emit((char*)"=[]",names,tmpArrVal,tmp);
			strcpy(midValue,tmp);/////���ӵķ���ֵ����midValue,  ���ص���������+[tmpArrVal]
			getsym();
			return ;
       }else {
			res = searchTab(names,0,0);
			if (res==-1) {
				error(8);
				 skip(1);
				return;
			}
		}
		strcpy(midValue,names);
		return;
	}
	if (symbol==L_SMALL) {
		getsym();
		expression();
		if (symbol!=R_SMALL) {
			error(20);
			skip(0);
			return;
		}
		getsym();
		return;
	}
	// ��������|���ַ���
	//�������� ::= �ۣ������ݣ��޷�������������
	if (symbol==PLUS || symbol==MINUS|| symbol==integer||symbol==character) {
		if (symbol==PLUS) getsym();
		else if (symbol==MINUS) {
			int k=0;
			getsym();
			while (token[k++]!='\0');
			token[k+1] = '\0';
			while (k!=0) {
				token[k] = token[k-1];
				k--;
			}
			token[0] = '-';
		}
		strcpy(midValue,token);//������ֵ��midValue
		if(symbol==character){
            char charVal[30];
            itoa(int(token[0]),charVal,10);
            strcpy(midValue,charVal);
            printChar--;
		}
		getsym();
		return;
	}
	error(0);                  //���ʽȱʧ�����
	skip(0);
    return;
}

//��� ::= �����ӣ�{���˷�������������ӣ�}
void term(){
    char fact1[30],fact2[30],fact3[30];
	factor();
	strcpy(fact3,midValue);//���ֲ�����Ϊ�˶Ը�ֻ�и�ֵ�����,������޳˳�����ôp1��p2ûʲô��
	while (symbol==MUL|| symbol==DIV) {
        printChar++;
        int tmpSym=symbol;
 		strcpy(fact1,fact3);////�г˳�ʱ��һ������������p1
 		getsym();
        factor();
        strcpy(fact2,midValue);/////�ڶ�������������p2
        strcpy(fact3,tempVal());//������ʱ����
		if (tmpSym==MUL)
			emit((char*)"imul",fact1,fact2,fact3);
		if (tmpSym==DIV)
			emit((char*)"idiv",fact1,fact2,fact3);
        continue;
	}
	strcpy(midValue,fact3);  //ÿһ���������ֵ����midValue����
	return;
}

//�����ʽ�� ::= �ۣ������ݣ��{���ӷ�����������}
void expression(){
    char term1[30],term2[30],term3[30];
	if (symbol==PLUS || symbol==MINUS) {
        printChar++;
		if (symbol==PLUS) {
			getsym();
			term();
			strcpy(term3,midValue);
		}
		if (symbol==MINUS) {
			getsym();
			term();
			strcpy(term1,midValue);
			strcpy(term3,tempVal());//place3 Ϊ��ʱ����,tempVal ������ʱ������$��ͷ
			emit((char*)"sub",(char*)"0",term1,term3);
		}
	}
	else {
		term();
		strcpy(term3,midValue);
	}
	while (symbol==PLUS || symbol==MINUS) {
        printChar++;
		strcpy(term1,term3);//////����һ�����������õ��Ľ����Ϊ�µ�Դ������1
		if (symbol==PLUS) {
			getsym();
			term();
			strcpy(term2,midValue);////��ǰ��ļ�����
			strcpy(term3,tempVal());//�³��µ���ʱ����
			emit((char*)"add",term1,term2,term3);//��������ļӷ��������浽�µ���ʱ������
			continue ;
		}
		if (symbol==MINUS) {
			getsym();
			term();
			strcpy(term2,midValue);
			strcpy(term3,tempVal());
			emit((char*)"sub",term1,term2,term3);
			continue ;
		}
	}
	strcpy(midValue,term3);//�ѱ��ʽ������ֵ�����midValue֮��
	return;
}

//��������    ::=  �����ʽ������ϵ������������ʽ���������ʽ�� //���ʽΪ0����Ϊ�٣�����Ϊ��
void ifLabel(char* label){
   char express1[30],express2[30];
	expression();
	strcpy(express1,midValue);  //����������һ�����ʽ
	int tmpSym1=symbol;
	if (tmpSym1==LESS || tmpSym1==G_EQUAL || tmpSym1==GREATER || tmpSym1==L_EQUAL || tmpSym1==NOT_EQUAL || tmpSym1==D_EQUAL) {
        getsym();
        expression();
        strcpy(express2,midValue);
		if (tmpSym1==LESS)
			emit((char*)"jge",express1,express2,label);
		else if (tmpSym1==L_EQUAL)
			emit((char*)"jnle",express1,express2,label);
		else if (tmpSym1==GREATER)
			emit((char*)"jle",express1,express2,label);
		else if (tmpSym1==G_EQUAL)
			emit((char*)"jnge",express1,express2,label);
		else if (tmpSym1==NOT_EQUAL)
			emit((char*)"jz",express1,express2,label);
		else if (tmpSym1==D_EQUAL)
			emit((char*)"jnz",express1,express2,label);
		return;
	}
	strcpy(express2,"0");
	emit((char*)"jz",express1,express2,label);/////�����ʽ��ֵ�ǲ��ǵ���0��Ϊ0��ת
	return;
}

//��������䣾  ::=  if ��(������������)������䣾��else����䣾��
void ifStatement(){
    char label1[30],label2[30];
    char ifLabelvalue[30];
    strcpy(label1,createLabel());
    strcpy(label2,createLabel());
    if(checkNextSym(1,L_SMALL,19,2))return;
    getsym();
    ifLabel(label1);
    strcpy(ifLabelvalue,midValue);
    if(checkNextSym(0,R_SMALL,20,2))return;
    getsym();
    statement();
    emit((char*)"jmp",(char*)" ",(char*)" ",label2);
    emit((char*)"label",(char*)" ",(char*)" ",label1);
    if(symbol==ELSE){
        getsym();
        statement();
    }
    emit((char*)"label",(char*)" ",(char*)" ",label2);
    return ;
}

//��ѭ����䣾
void doStatement(){
	char label1[10],label2[10];
	strcpy(label1,createLabel());
	strcpy(label2,createLabel());
    getsym();
    emit((char*)"label",(char*)" ",(char*)" ",label1);//������ת�����ı�ǩ
    statement();
    if(checkNextSym(0,WHILE,26,2))return;
    if(checkNextSym(1,L_SMALL,19,2))return;
    getsym();
    ifLabel(label2);//����Ϊ��ʱ����label2
    if(checkNextSym(0,R_SMALL,18,2))return;
    emit((char*)"jmp",(char*)" ",(char*)" ",label1);
    emit((char*)"label",(char*)" ",(char*)" ",label2);
    getsym();
    return;
}

void forStatement(){
	char label1[10],label2[10],names1[30],names2[30],tmp_num[30],tmp_mid[30];
	int res,sign;
	strcpy(label1,createLabel());
	strcpy(label2,createLabel());
	if(checkNextSym(1,L_SMALL,19,2))return;
	if(checkNextSym(1,ident,0,2))return;
	res = searchTab(token,0,0);
    if (res<0||Table[res].value!=0||Table[res].size!=0) {//�Ҳ������������ҵ����Ǹ����������Ǹ��������
        error(12);            //"="����ǲ��Ϸ��ı�ʶ��
        skip(1);
        return ;
    }
    strcpy(names1,token);
	if(checkNextSym(1,EQUAL,10,2))return;
    getsym();
    expression();
    emit((char*)"mov", midValue,(char*)" ",names1);
    if(checkNextSym(0,SEMICOLON,15,2))return;
    getsym();
    emit((char*)"label",(char*)" ",(char*)" ",label1);
    ifLabel(label2);
    if(checkNextSym(0,SEMICOLON,15,2))return;
    if(checkNextSym(1,ident,0,2))return;
    if(checkNextSym(1,EQUAL,10,2))return;
    if(checkNextSym(1,ident,0,2))return;
    strcpy(names2,token);
    getsym();
    if(symbol!=PLUS && symbol!=MINUS){
        error(17);
        skip(2);
        return;
    }
    sign=symbol;
    if(checkNextSym(1,integer,12,2))return;
    strcpy(tmp_num,token);
    if(checkNextSym(1,R_SMALL,20,2))return;
    getsym();
    statement();
    strcpy(tmp_mid,tempVal());
    if(sign==PLUS)emit((char*)"add",names2,tmp_num,tmp_mid);
    else  emit((char*)"sub",names2,tmp_num,tmp_mid);
    emit((char*)"mov", tmp_mid,(char*)" ",names1);
    emit((char*)"jmp",(char*)" ",(char*)" ",label1);
    emit((char*)"label",(char*)" ",(char*)" ",label2);
}

//������䣾    ::=  scanf ��(������ʶ����{,����ʶ����}��)��
void readStatement(){
    char names[30];
	int res;
	if(checkNextSym(1,L_SMALL,19,2))return;
	do{
        if(checkNextSym(1,ident,0,2))return;
		strcpy(id,token);
		strcpy(names,id);
		res = searchTab(names,0,0);
		if (res==-1 || Table[res].type==functype||Table[res].size!=0) {
			error(8);            //���Ϸ��ı�ʶ��
			skip(2);
	    	return;
		}
		if(Table[res].type==inttype)
		emit((char*)"scanf",(char*)"int",(char*)" ",names);
		else if(Table[res].type==chartype)
        emit((char*)"scanf",(char*)"char",(char*)" ",names);
		getsym();
	}while(symbol==COMMA);
	if(checkNextSym(0,R_SMALL,20,2))return;
	getsym();
	return;
}

//��д��䣾    ::= printf ��(�� ���ַ�����,�����ʽ�� ��)��| printf ��(�����ַ����� ��)��| printf ��(�������ʽ����)��
void writeStatement(){
    char str1[200]="\0",express1[30]=" ";
    if(checkNextSym(1,L_SMALL,19,2))return;
	getsym();
	if (symbol==text) {
		strcpy(str1,token);
		getsym();
		if (symbol==COMMA) {
			getsym();
			printChar=0;
			expression();
			strcpy(express1,midValue);
		 }
    }else {
        printChar=0;
		expression();
		strcpy(express1,midValue);
    }
    if(checkNextSym(0,R_SMALL,20,2))return;
    if(printChar==-1 && strcmp(express1," ")!=0){
        emit((char*)"printf",str1,express1,(char*)"1");
    }
	else
	    emit((char*)"printf",str1,express1,(char*)" ");
	printChar=0;
	getsym();
	return;
}

//��������䣾   ::=  return[��(�������ʽ����)��]
void returnStatement(){
   char express[30];
   getsym();
   if(symbol==L_SMALL){
    getsym();
    expression();
    strcpy(express,midValue);
    if(checkNextSym(0,R_SMALL,20,2))return;
      emit((char*)"ret",(char*)" ",(char*)" ",express);
      hasReturn++;
      getsym();
   }else emit((char*)"ret",(char*)" ",(char*)" ",(char*)" ");
   return ;
}

//��ֵ������   ::= �����ʽ��{,�����ʽ��}�����գ�
void valParameter(){
    char paras[100][100];
    int j = 0;
    do {
		if (symbol==COMMA) getsym();
		if (symbol==PLUS || symbol==MINUS || symbol==ident ||symbol==L_SMALL || symbol==character||symbol==integer) {
			expression();
			strcpy(paras[j],midValue);
			j++;
		}
	}while (symbol==COMMA);
	paraNums = j;
	for(j=0;j<paraNums;j++)
        emit((char*)"para",(char*)" ",(char*)"1",paras[j]);
	return ;
}

void statement(){
    int res;
    if(symbol==SEMICOLON){
        return;
    }
    if(symbol==IF){
        ifStatement();
        return;
    }
	if (symbol==DO) {
		doStatement();
		return;
	}
	if (symbol==FOR) {
		forStatement();
		return;
	}
    if (symbol==L_BIG) {
        getsym();
        stateList();
        if(checkNextSym(0,R_BIG,22,1))return;
        getsym();
        return;
    }
    if (symbol==SCANF) {
        readStatement();
        if(checkNextSym(0,SEMICOLON,15,1))return;
        getsym();
        return;
    }
    if (symbol==PRINTF) {
        writeStatement();
        if(checkNextSym(0,SEMICOLON,15,1))return;
        getsym();
        return;
    }
    if (symbol==RETURN) {
        returnStatement();
        if(checkNextSym(0,SEMICOLON,15,1))return;
        getsym();
        return;
    }
	if (symbol==ident) {
		char names[30];
		strcpy(id,token);
		strcpy(names,id);
		getsym();
		//����ֵ��䣾   ::=  ����ʶ�����������ʽ��|����ʶ������[�������ʽ����]��=�����ʽ��
		if (symbol==EQUAL) {
			res = searchTab(names,0,0);
			if (res<0||Table[res].type==constchartype||Table[res].type==constinttype||Table[res].size!=0) {//�Ҳ������������ҵ����Ǹ����������Ǹ��������
				error(8);
				skip(1);
                return ;
			}
			getsym();
			expression();
			emit((char*)"mov", midValue,(char*)" ",names);
			if(checkNextSym(0,SEMICOLON,15,1))return;
			getsym();
			return;
		}
		if(symbol==L_MIDDLE){
            res = searchTab(names,0,1);
            if(res<0||Table[res].size==0){//�Ҳ����������߱������Ǹ��������
                error(8);
				skip(1);
                return ;
            }
            int arrLength=Table[res].size;
            getsym();
            expression();
            char tmpArrVal[30];
            strcpy(tmpArrVal,midValue);
            if(tmpArrVal[0]>='0'&&tmpArrVal[0]<='9'){//���tmpArrVal�Ǹ����֣�Ҫ�������Խ��
                int a;
                a=atoi(tmpArrVal);
                if(a>=arrLength){
                    error(13);
                    skip(1);
                    return ;
                }
            }
            if(checkNextSym(0,R_MIDDLE,18,1))return;
            if(checkNextSym(1,EQUAL,10,1))return;
			getsym();
			expression();
			char tempplace[30];
			strcpy(tempplace,tempVal());
			emit((char*)"[]=",tmpArrVal,midValue,names);
			if(checkNextSym(0,SEMICOLON,15,1))return;
			getsym();
			return;
		  }
		//�����������
		if (symbol==L_SMALL) {
			getsym();
			valParameter();
			if(checkNextSym(0,R_SMALL,20,1))return;
			res = searchTab(names,1,0);
			if (res==-1 || res==-3||Table[res].type!=functype) {
                error(8);
				skip(1);
				return;
			}
			if(Table[res].size==0)
			emit((char*)"call",names,(char*)"int",(char*)" ");
			else if(Table[res].size==1)
            emit((char*)"call",names,(char*)"char",(char*)" ");
            else
            emit((char*)"call",names,(char*)" ",(char*)" ");
            if(checkNextSym(1,SEMICOLON,15,1))return;
			getsym();
		}
		else {
			error(0);    //���Ϸ��ľ���
			skip(1);
            return;
		}
		return;
	}
	if(symbol==SEMICOLON){
       getsym();
       return ;
    }
    else if(symbol==R_BIG){
        return;
    }
    error(0);
    skip(1);
    return;
}

void stateList(){
    statement();
    while(symbol==IF||symbol==ELSE||symbol==DO||symbol==FOR||symbol==SCANF||symbol==PRINTF
          ||symbol==L_BIG||symbol==ident||symbol==RETURN||symbol==SEMICOLON){
            if(symbol!=SEMICOLON)statement();
            else getsym();
    }
}

//��������䣾   ::=  �ۣ�����˵�����ݣۣ�����˵�����ݣ�����У�
void comStatement(){
    tempSym=0;
    if(symbol==CONST){
        constDeclare();
    }
    while(tempSym==SEMICOLON){
        if(symbol==CONST)
            constDeclare();
        else break;
    }
    while(symbol==INT||symbol==CHAR){
        int a=ftell(fin);//��¼�µ�ǰ�������ļ�λ�ã��Ա�Ԥ�����󷵻ط���
        int tmpSym2=symbol;
        if(symbol==INT) type=inttype;
        if(symbol==CHAR) type = chartype;
        getsym();
        if(symbol==ident){
            strcpy(id,token);
            getsym();
            if(symbol==COMMA||symbol==L_MIDDLE){
                symbol=tmpSym2;
                fseek(fin,a,SEEK_SET);//��������
                varDeclare();
                continue;
            }
            else if(symbol==SEMICOLON){
                insertTable(id,type,0,0,0,tabIndex,level);
                if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
                getsym();
                continue;
            }
            else {
                error(0);
                skip(1);
                return;
            }
        }
    }
    stateList();
    return;
}

//������ ::= �ۣ�����˵�����ݣۣ�����˵������{���з���ֵ�������壾|���޷���ֵ�������壾}����������
void procedure(){
    level=0;
    getsym();
    if(symbol==CONST)
        constDeclare();
    while(tempSym==SEMICOLON){
        if(symbol==CONST)
            constDeclare();
        else break;
    }
    while(symbol==INT||symbol==CHAR){
        int a=ftell(fin);//��¼�µ�ǰ�������ļ�λ�ã��Ա�Ԥ�����󷵻ط���
        int tmpSym2=symbol;
        int tmpLineNum=lineNum;
        if(symbol==INT) type=inttype;
        if(symbol==CHAR) type = chartype;
        getsym();
        if(symbol==ident){
            strcpy(id,token);
            getsym();
            if(symbol==COMMA||symbol==L_MIDDLE){
                symbol=tmpSym2;
                fseek(fin,a,SEEK_SET);//��������
                lineNum=tmpLineNum;
                varDeclare();
                continue;
            }else if(symbol==SEMICOLON){
                insertTable(id,type,0,0,0,tabIndex,level);
                if(type==inttype) emit((char*)"int",(char*)" ",(char*)" ",id);
                else if(type==chartype) emit((char*)"char",(char*)" ",(char*)" ",id);
                getsym();
                continue;
            }else {//���������ʵֻʣ��һ�֣�С���ŵ������Ϊ�з���ֵ�������壬���������к����ķ���
                symbol=tmpSym2;
                lineNum=tmpLineNum;
                fseek(fin,a,SEEK_SET);
                break;
            }
        }
    }
    if(symbol!=INT&&symbol!=CHAR&&symbol!=VOID){
        error(0);
        skip(5);
    }

//���з���ֵ�������壾  ::=  ������ͷ������(������������)�� ��{����������䣾��}��
    while(symbol==INT||symbol==CHAR||symbol==VOID){
        level=0;
        int tmpSym3=symbol;
         if(symbol==INT||symbol==CHAR){
               if(symbol==INT)size=0;
               if(symbol==CHAR)size=1;
               defineHead();
               if(checkNextSym(0,L_SMALL,19,5))continue;
             funcTmpNum=0;
             hasReturn=0;
             type=functype;
             insertTable(id,type,1,size,0,tabIndex,level);
             level=1;
             strcpy(proName,id);
             if(tmpSym3==INT)
                emit((char*)"start",(char*)"int",(char*)" ",proName);
             if(tmpSym3==CHAR)
                emit((char*)"start",(char*)"char",(char*)" ",proName);
             getsym();
             parameters();
             if(checkNextSym(0,R_SMALL,20,5))continue;
             if(checkNextSym(1,L_BIG,21,5))continue;
             getsym();
             comStatement();
             if(checkNextSym(0,R_BIG,22,5))continue;
             if(hasReturn==0){
                error(23);
                skip(5);
				continue;
             }
             getsym();
             emit((char*)"end",(char*)" ",(char*)" ",proName);
             clearFuncTab();//��պ�������ķ��ű�
             level=0;
           }
        else if(symbol==VOID){
            level=0;
            getsym();
            if(symbol==MAIN){
               strcpy(id,"main");
               funcTmpNum=0;
               type=functype;
               size=-1;
               insertTable(id,type,0,size,0,tabIndex,level);
                level=1;
               strcpy(proName,id);
               emit((char*)"start",(char*)" ",(char*)" ",proName);
               if(checkNextSym(1,L_SMALL,19,5))continue;
               if(checkNextSym(1,R_SMALL,20,5))continue;
               if(checkNextSym(1,L_BIG,21,5))continue;
               getsym();
               comStatement();
               if(checkNextSym(0,R_BIG,22,5))continue;
             hasMain++;
             emit((char*)"end",(char*)" ",(char*)" ",proName);
             level=0;
             return;
          }
          //���޷���ֵ�������壾  ::= void����ʶ������(������������)����{����������䣾��}��
          if(checkNextSym(0,ident,0,5))continue;
             level=0;
             funcTmpNum=0;
             type=functype;
             hasReturn=0;
             size=-1;
             strcpy(id,token);
             insertTable(id,type,0,size,0,tabIndex,level);
             level=1;
             strcpy(proName,id);
             emit((char*)"start",(char*)"void",(char*)" ",proName);
             if(checkNextSym(1,L_SMALL,19,5))continue;
             getsym();
             parameters();
             if(checkNextSym(0,R_SMALL,20,5))continue;
             if(checkNextSym(1,L_BIG,21,5))continue;
             getsym();
             comStatement();
             if(checkNextSym(0,R_BIG,22,5))continue;
             if(hasReturn==1){
                error(24);
                skip(5);
				continue;
             }
             getsym();
             emit((char*)"end",(char*)" ",(char*)" ",proName);
             clearFuncTab();
             level=0;
          }
        }
    if(hasMain==0){
        error(25);
        return;
    }
}

