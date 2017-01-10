#include "define.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

using namespace std;

extern table Table[200];
extern quaternary code[1000];
extern int codeNum;
int index=0;
int counter=0;//�ֲ���������ʱ��������
int paraNum=0;
int paraCounter = 0;
int funcTyp=-1;
int tmpLabel=0;
varNode *localList = NULL,*tempList = NULL,*globalList=NULL,*tp,*read;
FILE* asmFile;

void asmHead(char disk){
    fprintf(asmFile,".386\n");
    fprintf(asmFile,".model flat,stdcall\n");
    fprintf(asmFile,"option casemap:none\n");
    fprintf(asmFile,"Include %c:\\masm32\\include\\windows.inc\n",disk);
    fprintf(asmFile,"Include %c:\\masm32\\include\\kernel32.inc\n",disk);
    fprintf(asmFile,"Include %c:\\masm32\\include\\msvcrt.inc\n",disk);
    fprintf(asmFile,"Includelib %c:\\masm32\\lib\\msvcrt.lib\n",disk);
    fprintf(asmFile,"Includelib %c:\\masm32\\lib\\kernel32.lib\n",disk);
    fprintf(asmFile,"Include %c:\\masm32\\macros\\macros.asm\n\n",disk);
}

varNode* search(varNode * head, char *id){
	varNode *t = head;
	while(t != NULL){
		if(strcmp(t->id,id) == 0)
			return t;
		t = t->next;
	}
	return NULL;
}

void initList(varNode* head){
	if(head == NULL)
		return;
	else{
		initList(head->next);
		free(head);
	}
}

void getArrIndex(char *s){
    char name[30];
    strcpy(name,s);
    read = search(localList,name);
    if(read == NULL)read =search(tempList,name);
    if(read != NULL)
        fprintf(asmFile,"\tmov\tebx,\tdword  ptr [ebp+(%d)]\n",read->add);
    else{
        read = search(globalList,name);
        if(read != NULL)//ȫ�ֻ��߳���
            fprintf(asmFile,"\tmov\tebx,\t_%s\n",name);
        else
            fprintf(asmFile,"\tmov\tebx,\t%s\n",name);
    }
}

int getArrBase(char* s,int fb){
    char name[30];
    int res=-1;
    strcpy(name,s);
    strcat(name,"[0]");
    read = search(localList,name);
    if(read!=NULL)
        res=read->add;//�Ҳ����Ļ�˵���Ǹ�ȫ�ֶ��������
    if(res==-1){
        if(fb==1)
            fprintf(asmFile,"\tmov\teax,\tdword ptr[%s+4*ebx]\n",s);
        else
            fprintf(asmFile,"\tmov\tdword ptr[%s+4*ebx] ,eax\n",s);
        return -1;
    }
    else{
        fprintf(asmFile,"\tneg ebx\n");
        if(fb==1)
            fprintf(asmFile,"\tmov\teax,\tdword ptr[ebp+(%d)+4*ebx]\n",res);
        else
            fprintf(asmFile,"\tmov\tdword ptr[ebp+(%d)+4*ebx] ,eax\n",res);
        return 0;
    }
}

void addLink(varNode **target,char *id,int add,int typ){
    tp = (varNode*)malloc(sizeof(varNode));
    tp->add = add;
    tp->next = NULL;
    strcpy(tp->id,id);
    tp->type = typ;
    if(*target == NULL)
        *target = tp;
    else{
        tp->next=(*target)->next;
        (*target)->next=tp;
    }
}

bool codeOp(char *s){
    return (strcmp(code[index].op,s)==0);
}

bool codeVar1(char *s){
    return (strcmp(code[index].var1,s)==0);
}

bool codeVar2(char *s){
    return (strcmp(code[index].var2,s)==0);
}

bool codeVar3(char *s){
    return (strcmp(code[index].var3,s)==0);
}

void pushNew(int t){
    if(!codeVar2(" ")){
        int arrLength = atoi(code[index].var2);
        if(t==1)
            fprintf(asmFile,"\t%s\tDWORD\t%d DUP(?)\n",code[index].var3,arrLength);
        for(int i=0;i<arrLength;i++){
            char arrIndex[20],tmpName[20];
            int tmpTyp=0;
            strcpy(tmpName,code[index].var3);
            itoa(i,arrIndex,10);
            strcat(tmpName,"[");
            strcat(tmpName,arrIndex);
            strcat(tmpName,"]");
            if(codeOp("char"))
                tmpTyp=1;
            if(t==1)
                addLink(&globalList,tmpName,0,tmpTyp);
            else{
                addLink(&localList,tmpName,(-1)*4*(++counter),tmpTyp);
                fprintf(asmFile,"\tmov\teax,\t0;LocalVar\n");//������ֵĬ��Ϊ0
                fprintf(asmFile,"\tpush\teax\n");//�ֲ�����ѹջ
            }
        }
    }else{
        int tmpTyp=0;
        if(codeOp("char"))
            tmpTyp=1;
        if(t==1){
            fprintf(asmFile,"\t_%s\tdword\t?\n",code[index].var3);
            addLink(&globalList,code[index].var3,0,tmpTyp);
        }else{
            addLink(&localList,code[index].var3,(-1)*4*(++counter),tmpTyp);
            fprintf(asmFile,"\tmov\teax,\t0;LocalVar\n");//������ֵĬ��Ϊ0
            fprintf(asmFile,"\tpush\teax\n");//�ֲ�����ѹջ
        }
    }
}

void newTmpVal(int add,int tmpTyp,char *s){
    add=(-4)*(++counter);
    addLink(&tempList,s,add,tmpTyp);
    fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",add);
    fprintf(asmFile,"\tmov   ebx,ebp\n");//��ʱ���������ڴ���
    fprintf(asmFile,"\tadd   ebx,%d\n",add);
    fprintf(asmFile,"\tcmp   esp ,ebx\n");
    fprintf(asmFile,"\tjle  espp%d\n",tmpLabel);
    fprintf(asmFile,"\tmov esp, ebx\n");
    fprintf(asmFile,"espp%d:\n",tmpLabel++);
}

void findAllList(int i){
    char s[30];
    if(i==1)strcpy(s,code[index].var1);
    else if(i==2)strcpy(s,code[index].var2);
    else if(i==3)strcpy(s,code[index].var3);
    read = search(localList,s);
    if(read == NULL)read = search(tempList,s);
    if(read != NULL)
        fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
    else{
        read=search(globalList,s);
        if(read==NULL){//���Ҳ����ǳ���
            int arrSize,i;
            char temp[30];
            strcpy(temp,s);
            arrSize = strlen(temp);
            for(i=0;i<arrSize;i++)
                if(temp[i]=='[')
                    break;
            if(i==arrSize){
                if(s[0]=='$')
                    newTmpVal((-4)*(++counter),0,s);
                else if((s[0]>='0'&&s[0]<='9')||((s[0]=='-'||s[0]=='+')&&s[1]>='0'&&s[1]<='9'))
                    fprintf(asmFile,"\tmov\teax,\t%s\n",s);
                else
                    fprintf(asmFile,"\tmov\teax,\t\'%s\'\n",s);
            }
            else{//change[$0]
                char arrIndex[30];
                int j,k=0;
                for(j=i+1;j<arrSize;j++)
                    if(temp[j]!=']')
                        arrIndex[k++]=temp[j];
                arrIndex[k]='\0';
                getArrIndex(arrIndex);
                for(;i<arrSize;i++)
                    temp[i]='\0';
                getArrBase(temp,1);
            }
        }
        else{//��ȫ�ֱ���
            int arrSize,i;
            char temp[30];
            strcpy(temp,s);
            arrSize = strlen(temp);
            for(i=0;i<arrSize;i++)
                if(temp[i]=='[')
                    break;
            if(i==arrSize)
                fprintf(asmFile,"\tmov\teax,\t_%s\n",temp);
            else {//����
                char arrIndex[30];
                int j,k=0;
                for(j=i+1;j<arrSize;j++){
                    if(temp[j]!=']')
                    arrIndex[k++]=temp[j];
                }
                arrIndex[k]='\0';
                getArrIndex(arrIndex);
                for(;i<arrSize;i++)
                    temp[i]='\0';
                getArrBase(temp,1);
            }
        }
    }
}

void dataSeg(){
    fprintf(asmFile,".data\n");
    while (codeOp("const")) {
        int tmpTyp=0;
        if (codeVar1("int"))
            fprintf(asmFile,"\t_%s\tequ\t%s\n",code[index].var3,code[index].var2);
        else{
            fprintf(asmFile,"\t_%s\tequ\t\'%s\'\n",code[index].var3,code[index].var2);
            tmpTyp=1;
        }
        addLink(&globalList,code[index++].var3,0,tmpTyp);
	}
	while(codeOp("int")||codeOp("char")){
        pushNew(1);
        index++;
	}
	fprintf(asmFile,"\n");
}

void codeSeg(){
    char funcName[20];
    fprintf(asmFile,".CODE\n");
    while(index<codeNum){
        if(codeOp("label"))
            fprintf(asmFile,"%s:\n",code[index].var3);
        else if(codeOp("jmp"))
            fprintf(asmFile,"\tjmp\t%s\n",code[index].var3);
        else if(codeOp("int")||codeOp("char"))
			pushNew(0);
        else if(codeOp("start")){
            counter = 0;
            paraNum = 0;
            strcpy(funcName,code[index].var3);
            if(codeVar3("main")){
                fprintf(asmFile,"START:\n");
                fprintf(asmFile,"\tpush\tebp\n");
                fprintf(asmFile,"\tmov\tebp,\tesp\n");
            }else{
                int tmpIndex = index+1;
                if(codeVar1("char"))
                    funcTyp=1;
                if(codeVar1("int"))
                    funcTyp=0;
                if(codeVar1("void"))
                    funcTyp=2;
                fprintf(asmFile,"_%s\tPROC\n",code[index].var3);
                while(strcmp(code[tmpIndex].op,"para")==0 && strcmp(code[tmpIndex].var2,"0")==0){
                    paraNum++;
                    tmpIndex++;
                }
                fprintf(asmFile,"\tpush\tebx;saveState\n");
                fprintf(asmFile,"\tpush\tebp\n");//paraCounter	ebp  ���ú�����ebp��֮ǰ��һ����retadress
                fprintf(asmFile,"\tmov\tebp,\tesp;saveState finish\n");// mov ebp,esp  EBPָ����ǵ�ǰ�����Ļ���ַ
            }
        }
        else if(codeOp("end")){
            if(codeVar3("main")){
                fprintf(asmFile,"   invoke ExitProcess, 0; exit with code 0\n");
                fprintf(asmFile,"END START\n");
            }else{
                fprintf(asmFile,"\tmov\tesp,\tebp\n");
                fprintf(asmFile,"\tpop\tebp\n");
                fprintf(asmFile,"\tpop\tebx\n");
                fprintf(asmFile,"\tret\n");
                counter = 0;
                initList(localList);
                localList = NULL;
                initList(tempList);
                tempList = NULL;
                fprintf(asmFile,"_%s\tendp\n",funcName);
            }
        }
        else if(codeOp("para")){
            if(codeVar2("0")){
                int tmpTyp=0;
                if(codeVar1("char"))
                    tmpTyp=1;
                addLink(&localList,code[index].var3,4 * (2 + paraNum--),tmpTyp);
            }else{
                paraCounter++;
                findAllList(3);
                fprintf(asmFile,"\tpush\teax;pass value to function\n");//����ѹջ
            }
        }
        else if(codeOp("const")){
            int tmpTyp=0;
            if (codeVar1("int")){
                fprintf(asmFile,"\tmov\teax,\t%s;constVar\n",code[index].var2);
            }else{
                fprintf(asmFile,"\tmov\teax,\t\'%s\';constVar\n",code[index].var2);
                tmpTyp=1;
            }
            addLink(&localList,code[index].var3,(-1)*4*(++counter),tmpTyp);
            fprintf(asmFile,"\tpush\teax\n");
        }
		else if(codeOp("add")||codeOp("sub")|| codeOp("imul") || codeOp("idiv")){
            int tmpTyp=0,add;
            findAllList(2);
			fprintf(asmFile,"\tmov\tebx,\teax\n");//��2��������������ebx
			// ��1��Դ������
			findAllList(1);
			if(codeOp("add")|| codeOp("sub"))
                fprintf(asmFile,"\t%s\teax,\tebx\n",code[index].op);
			else{
                fprintf(asmFile,"\tcdq\n");
                fprintf(asmFile,"\t%s\tebx\n",code[index].op);
			}
			// ���������������ֵ
			read = search(localList,code[index].var3);
			if(read == NULL)read = search(tempList,code[index].var3);
			if(read != NULL)
                fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",read->add);
            else{
                read = search(globalList,code[index].var3);
                if(read == NULL)
                    newTmpVal((-4)*(++counter),tmpTyp,code[index].var3);
                else//Ϊȫ�ֱ���
                    fprintf(asmFile,"\tmov _%s,\teax\n",code[index].var3);
            }
        }
        else if(codeOp("scanf")) {
			read = search(localList,code[index].var3);
			if(read == NULL)read = search(tempList,code[index].var3);
			if(read != NULL){
                if(codeVar1("int"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%d\"),addr dword ptr[ebp+(%d)]\n",read->add);
                else if(codeVar1("char"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%c\"),addr dword ptr[ebp+(%d)]\n",read->add);
			}else{
			    if(codeVar1("int"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%d\"),addr _%s\n",code[index].var3);
                else if(codeVar1("char"))
                    fprintf(asmFile,"\tinvoke crt_scanf,SADD(\"%%c\"),addr _%s\n",code[index].var3);
			}
		}
        else if(codeOp("printf"))  {
            fprintf(asmFile,"\tpush eax;print begin\n");
			if(!codeVar1("\0")){//���ַ���������ȴ�ӡ�ַ���
                char buf[100];
                strcpy(buf,code[index].var1);
                fprintf(asmFile,"invoke crt_printf,ADDR literal(\"%s\")\n",buf);
			}
			if(codeVar3("1")){
                char charOut[2]={char(atoi(code[index].var2))};
                fprintf(asmFile,"invoke crt_printf,ADDR literal(\"%s\")\n",charOut);
			}
			else if(!codeVar2(" ")){//��ӡ�Ķ����б��ʽ
				read = search(localList,code[index].var2);//�ھֲ�����������
				if(read == NULL)read = search(tempList,code[index].var2);
				if(read != NULL){
                    fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
                    if(read->type==0)
                        fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%d \"),eax\n");
                    if(read->type==1)
                        fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%c \"),eax\n");
				}else{//�ھֲ���������ʱ���������Ҳ�����Ҫô��ȫ�ֱ���Ҫô�����顣
				    read = search(globalList,code[index].var2);
                    if(read == NULL){
                        if((code[index].var2[0]>='0'&&code[index].var2[0]<='9') ||
                           ((code[index].var2[0]=='-'||code[index].var2[0]=='+')&& code[index].var2[1]>='0'&&code[index].var2[1]<='9')){
                            fprintf(asmFile,"\tmov\teax,\t%s\n",code[index].var2);
                            fprintf(asmFile,"invoke crt_printf,SADD(\"%%d \"),eax\n");
                        }
                        else{
                            fprintf(asmFile,"\tmov\teax,\t\'%s\'\n",code[index].var2);
                            fprintf(asmFile,"invoke crt_printf,SADD(\"%%c \"),eax");
                        }
                    }else{
                        int arrSize,i;
                        char temp[30];
                        strcpy(temp,code[index].var2);
                        arrSize = strlen(temp);
                        for(i=0;i<arrSize;i++)
                            if(temp[i]=='[')
                            break;
                        if(i==arrSize){
                            fprintf(asmFile,"\tmov\teax,\t_%s\n",code[index].var2);
                            if(read->type==0)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%d \"),eax\n");
                            if(read->type==1)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%c \"),eax\n");
                        }
                        else{//������
                            varNode* readtemp=read;
                            char arrIndex[30];
                            int j,k=0;
                            for(j=i+1;j<arrSize;j++){
                                if(temp[j]!=']')
                                arrIndex[k++]=temp[j];
                            }
                            arrIndex[k]='\0';
                            getArrIndex(arrIndex);
                            for(;i<arrSize;i++)
                                temp[i]='\0';
                            int d=getArrBase(temp,1);
                            strcat(temp,"[0]");
                            if(d==-1)//ȫ���������
                                readtemp = search(globalList,temp);
                            else
                                readtemp=search(localList,temp);
                            if(readtemp->type==0)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%d \"),eax\n");
                            if(readtemp->type==1)
                                fprintf(asmFile,"\tinvoke crt_printf,SADD(\"%%c \"),eax\n");
                        }
                    }
                }
			}
			fprintf(asmFile,"invoke crt_printf,SADD(\"%%c\"),10\n");
			fprintf(asmFile,"\tpop eax;print over\n");
		}
		else if(codeOp("call"))  {
		    char tempstr[30];
			strcpy(tempstr,code[index].var3);
			fprintf(asmFile,"\tcall\t_%s\n",code[index].var1);
			fprintf(asmFile,"\tadd\tesp,\t%d\n",4*paraCounter);
			paraCounter = 0;
			//��������ķ���ֵ����Ҫ��
			if(index<codeNum && strcmp(tempstr," ") != 0 ){//�������ص���ʱ�����������ɵģ���Ҫ��������ռ�
                int add=(-1)*4*(++counter);
                int tmpTyp=0;
                if(codeVar2("char"))
                      tmpTyp=1;
                newTmpVal(add,tmpTyp,tempstr);
			}
			else{
                index++;
                continue;
            }
		}
		else if(codeOp("mov"))  {
			int temptype=0;
			findAllList(1);
			//�ٰѼĴ����е����ݷŵ���3����������
			read = search(localList,code[index].var3);
			if(read == NULL)read = search(tempList,code[index].var3);
			if(read != NULL)
			    fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",read->add);
            else{
                read = search(globalList,code[index].var3);
                if(read != NULL) {
                    if(!codeVar2(" ")){//ȫ�ֶ��������
                        int arrSize,i;
                        char temp[30];
                        strcpy(temp,code[index].var3);
                        arrSize = strlen(temp);
                        for(i=0;i<arrSize;i++)
                            if(temp[i]=='[')
                                break;
                        for(;i<arrSize;i++)
                            temp[i]='\0';
                        getArrIndex(code[index].var2);
                        strcpy(code[index].var3,temp);
                        getArrBase(temp,2);
                    }
                    else //��ͨȫ�ֱ���
                        fprintf(asmFile,"\tmov\t_%s,\teax\n",code[index].var3);
                }
                else if(read == NULL)
                    newTmpVal((-1)*4*(++counter),0,code[index].var3);
            }
		}
		else if(codeOp("ret"))  {
            if(strcmp(funcName,"main")==0)
                fprintf(asmFile,"   invoke ExitProcess, 0; exit with code 0\n");
			if(!codeVar3(" "))  {
				read = search(localList,code[index].var3);
				if(read != NULL)// ����Ǿֲ�����
				    fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
				else{
					read = search(tempList,code[index].var3);
					if(read != NULL) {// ������ʱ����
					    fprintf(asmFile,"\tmov\teax,\tdword  ptr [ebp+(%d)]\n",read->add);
						read->type=funcTyp;
					}
					else{//ȫ��
					    read =  search(globalList,code[index].var3);
					    if(read==NULL)
                            fprintf(asmFile,"\tmov\teax,\t%s\n",code[index].var3);
                        else
                            fprintf(asmFile,"\tmov\teax,\t_%s\n",code[index].var3);
					}
				}
			}
            fprintf(asmFile,"\tmov\tesp,\tebp\n");
            fprintf(asmFile,"\tpop\tebp\n");
            fprintf(asmFile,"\tpop\tebx\n");
            if(!(strcmp(code[index+1].op,"end")==0&&strcmp(code[index+1].var3,"main")==0))
                fprintf(asmFile,"\tret\n");
            funcTyp=-1;//��������Ѻ����ķ�����������һ��
		}
        else if(codeOp("jnge")||codeOp("jle")||codeOp("jnle")||codeOp("jge") ||codeOp("jz") ||codeOp("jnz") )  {
            findAllList(2);
            fprintf(asmFile,"\tmov\tebx,\teax\n");
            findAllList(1);
            fprintf(asmFile,"\tcmp\teax,\tebx\n");
            fprintf(asmFile,"\t%s\t%s\n",code[index].op,code[index].var3);
		}
		else if(codeOp("[]=")){
		    findAllList(2);
            getArrIndex(code[index].var1);
            getArrBase(code[index].var3,2);
		}
		else if(codeOp("=[]")){
            getArrIndex(code[index].var2);
            getArrBase(code[index].var1,1);
            int t=0;
            if(read!=NULL)t=read->type;
		    read = search(localList,code[index].var3);
            if(read == NULL)read = search(tempList,code[index].var3);
            if(read != NULL)
                fprintf(asmFile,"\tmov\tdword  ptr [ebp+(%d)],\teax\n",read->add);
            else{
                read=search(globalList,code[index].var3);
                if(read==NULL)
                    newTmpVal((-4)*(++counter),t,code[index].var3);
                else//��ȫ�ֱ���
                    fprintf(asmFile,"\tmov\t_%s,\teax\n",code[index].var3);
            }
		}
        index++;
    }
}

void genAsm(){
    asmFile=fopen("target.asm","w+");
    asmHead('e');
    dataSeg();
    codeSeg();
    fclose(asmFile);
}
