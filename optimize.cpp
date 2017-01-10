#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

extern quaternary code[1000];
extern int codeNum;

int blockMark[500]={0};
flowDiagram flow[100];
int flowTop=0;
block blocks[500];
int blockNum=0;
block tmpB[500];
int tmpTop=0;
dagNode dag[500];//dagͼ�еĽڵ�
int dagTop=0;
tabNode nodeTab[500];//�ڵ���еĽڵ�
int nodeTop=0;
Conflict conTab[50];
int conTop=0;
quaternary newcode[1000];
int newcodelen=0;

int isConst(char *s){
	if ((s[0]<='9'&&s[0]>='0') || (s[0]=='-'&&s[1]<='9'&&s[1]>='0'))
        return 1;
	else return 0;
}

void constFold(){
    int i, t1,t2,temp;
	char res[30];
	for(i=0;i<codeNum;i++){
        if(strcmp(code[i].op,"add")==0 || strcmp(code[i].op,"sub")==0 || strcmp(code[i].op,"imul")==0 || strcmp(code[i].op,"idiv")==0)
            if (isConst(code[i].var1) && isConst(code[i].var2)){
                t1 = atoi(code[i].var1);
                t2 = atoi(code[i].var2);
                if(strcmp(code[i].op,"add")==0) temp = t1+t2;
                else if(strcmp(code[i].op,"sub")==0) temp = t1-t2;
                else if(strcmp(code[i].op,"imul")==0) temp = t1*t2;
                else temp = t1/t2;
                itoa(temp,res,10);
                strcpy(code[i].op,"mov");
                strcpy(code[i].var1,res);
                strcpy(code[i].var2," ");
            }
        if(strcmp(code[i].op,"mov")==0 && isConst(code[i].var1) && code[i].var3[0]=='$' && i<codeNum-1){
            if(strcmp(code[i+1].var1,code[i].var3)==0)
                strcpy(code[i+1].var1,code[i].var1);
            else if(strcmp(code[i+1].var2,code[i].var3)==0)
                strcpy(code[i+1].var2,code[i].var1);
            else continue;
            for(int j=i;j<codeNum-1;j++){
                strcpy(code[j].op,code[j+1].op);
                strcpy(code[j].var1,code[j+1].var1);
                strcpy(code[j].var2,code[j+1].var2);
                strcpy(code[j].var3,code[j+1].var3);
            }
            codeNum--;
            i--;
        }
	}
}

void markBlockHead(){
	for(int i=0;i<codeNum;i++){
	    if(i==0 || strcmp(code[i].op,"label")==0)
            blockMark[i]=1;
		if( strcmp(code[i].op,"start")==0){
            blockMark[i]=1;
            blockMark[i+1]=1;
		}
		else if( strcmp(code[i].op,"jnge")==0||strcmp(code[i].op,"jle")==0||
                 strcmp(code[i].op,"jnle")==0||strcmp(code[i].op,"jge")==0||
                 strcmp(code[i].op,"jz")==0||strcmp(code[i].op,"jnz")==0||
                 strcmp(code[i].op,"jmp")==0||strcmp(code[i].op,"ret")==0)
            blockMark[i+1]=1;
		else{
			if( blockMark[i]!=1 )
				blockMark[i]=0;
		}
	}
}

void divideBlock(){
	for(int i=0;i<codeNum;i++){
		if(blockMark[i]==1){
            do{
                strcpy(blocks[blockNum].code[blocks[blockNum].codeNum].op,code[i].op);
			    strcpy(blocks[blockNum].code[blocks[blockNum].codeNum].var1,code[i].var1);
                strcpy(blocks[blockNum].code[blocks[blockNum].codeNum].var2,code[i].var2);
                strcpy(blocks[blockNum].code[blocks[blockNum].codeNum++].var3,code[i++].var3);
            }while((blockMark[i]!=1) && (i<codeNum));
			blockNum++;
		}
		i--;
	}
}

void buildFlow(){
    int readGlobal=0;
	for(int i=0;i<blockNum;i++){
	    while(blocks[i].codeNum>0 && strcmp(blocks[i].code[0].op,"start")!=0){
			flow[flowTop].block[flow[flowTop].blockNum]=i++;
			flow[flowTop].blockNum++;
			readGlobal=1;
	    }
	    if(readGlobal==1){
            flowTop++;
            readGlobal=0;
	    }
		if((blocks[i].codeNum==1)&&(strcmp(blocks[i].code[0].op,"start")==0)){
            do{
                flow[flowTop].block[flow[flowTop].blockNum]=i++;
                flow[flowTop].blockNum++;
            }while((!( (blocks[i].codeNum==1)&&(strcmp(blocks[i].code[0].op,"start")==0)))&&(i<blockNum));
			flowTop++;
			i--;
		}
	}
}

int searchBlock(int f,char s[]){
    for(int i=0;i<flow[f].blockNum;i++){
        quaternary a=blocks[flow[f].block[i]].code[0];
        if(strcmp(a.op,"label")==0&&strcmp(a.var3,s)==0)
            return i;
    }
	return 0;
}

void linkBlock(){
	for(int i=0;i<flowTop;i++){
		for(int j=0;j<flow[i].blockNum;j++){
            blocks[flow[i].block [j]].nextBlock[1]=NULL;
			if( j==(flow[i].blockNum-1) ){
				blocks[flow[i].block [j]].nextBlock [0]=NULL;
				break;
			}
			int k=blocks[flow[i].block[j]].codeNum-1;
			if(k<0)break;
            if( strcmp(blocks[flow[i].block[j]].code[k].op,"start")==0)
                blocks[flow[i].block[j]].nextBlock[0]=j+1;
            else if(strcmp(blocks[flow[i].block[j]].code[k].op,"jmp")==0)
                blocks[flow[i].block [j]].nextBlock[0]=searchBlock(i,blocks[flow[i].block[j]].code[k].var3);
            else if(strcmp(blocks[flow[i].block[j]].code[k].op,"jnge")==0 || strcmp(blocks[flow[i].block[j]].code[k].op,"jle")==0 ||
                    strcmp(blocks[flow[i].block[j]].code[k].op,"jnle")==0 || strcmp(blocks[flow[i].block[j]].code[k].op,"jge")==0 ||
                    strcmp(blocks[flow[i].block[j]].code[k].op,"jz")==0 || strcmp(blocks[flow[i].block[j]].code[k].op,"jnz")==0){
                blocks[flow[i].block [j]].nextBlock[0]=searchBlock(i,blocks[flow[i].block[j]].code[k].var3);
                blocks[flow[i].block [j]].nextBlock[1]=j+1;
            }
            else if(strcmp(blocks[flow[i].block[j]].code[k].op,"ret")==0)
                blocks[flow[i].block [j]].nextBlock[0]=flow[i].blockNum-1;
            else
                blocks[flow[i].block [j]].nextBlock[0]=j+1;
		}
	}
}

void clearDag(){
	for(int i=0;i<dagTop;i++){
		dag[i].nodeNum=NULL;
		dag[i].leftNum=NULL;
		dag[i].rightNum=NULL;
		dag[i].label=NULL;
		dag[i].isExported=0;
		strcpy(dag[i].labelName,"");
	}
	for(int i=0;i<nodeTop;i++){
		strcpy(nodeTab[i].nodeName,"");
		for(int j=0;j<100;j++)
		    nodeTab[i].nodeNumber[j]=NULL;
	}
	nodeTop=0;
	dagTop=0;
}

int findNode(char s[]){
	for(int i=0;i<nodeTop;i++)
		if(strcmp(s,nodeTab[i].nodeName)==0)
			return nodeTab[i].nodeNumber[nodeTab[i].counter-1];
	return -1;
}

int findOp(int x,int y,char s[]){
	for(int i=0;i<dagTop;i++)
		if( (x==dag[i].leftNum) && (y==dag[i].rightNum) && (strcmp(s,dag[i].labelName)==0))
			return dag[i].nodeNum;
	return -1;
}

void createDagNode(int left,int right,int lab){
    dag[dagTop].nodeNum=dagTop;
    dag[dagTop].leftNum=left;
    dag[dagTop].rightNum=right;
    dag[dagTop].label=lab;
}

void newNode(quaternary code,int typ){
    int x,y=NULL,z;
	int tmp,i;
	x=findNode(code.var1);
	if(x==-1){
        createDagNode(NULL,NULL,0);
		strcpy(dag[dagTop].labelName,code.var1);
		x=dagTop++;
		strcpy(nodeTab[nodeTop].nodeName,code.var1);
		nodeTab[nodeTop].nodeNumber[nodeTab[nodeTop].counter++]=x;
		nodeTop++;
	}
	if(typ==2){
        y=findNode(code.var2);
        if(y==-1){
            createDagNode(NULL,NULL,0);
            strcpy(dag[dagTop].labelName,code.var2);
            y=dagTop++;
            strcpy(nodeTab[nodeTop].nodeName,code.var2);
            nodeTab[nodeTop].nodeNumber[nodeTab[nodeTop].counter++]=y;
		    nodeTop++;
        }
	}
	tmp=findOp(x,y,code.op);
	if(tmp==-1){
	    createDagNode(x,y,1);//��Ҷ�ڵ�
		strcpy(dag[dagTop].labelName,code.op);
		tmp=dagTop++;
	}
	z=findNode(code.var3);
	if( z==-1 ){
		strcpy(nodeTab[nodeTop].nodeName,code.var3);
		nodeTab[nodeTop].nodeNumber[nodeTab[nodeTop].counter++]=tmp;
		nodeTop++;
	}
	else{//����ڽڵ�����ҵ�z����ôҪ����z�Ľڵ�ֵ
		for(i=0;i<nodeTop;i++)
			if( strcmp(code.var3,nodeTab[i].nodeName)==0){
                nodeTab[i].nodeNumber[nodeTab[i].counter++]=tmp;
			}
	}
}

int hasFather(int a){
    for(int i=0;i<dagTop;i++)
        if(dag[i].leftNum==a||dag[i].rightNum==a)
            if(dag[i].isExported==0)
                return 1;
    return 0;
}

int enqueue(){//�õ���һ����������еĽڵ��
	for(int i=0;i<dagTop;i++){
		if((dag[i].label==1)&&(dag[i].isExported==0)){//���ȣ�Ѱ���м�ڵ�
			if(hasFather(dag[i].nodeNum))
                continue;
			else
				return i;
		}
	}
	return -1;
}

char* getNodeName(int a){
	for(int i=0;i<nodeTop;i++)
        for(int j=0;j<nodeTab[i].counter;j++)
		    if(nodeTab[i].nodeNumber[j]==a)
			    return nodeTab[i].nodeName;
	return NULL;
}

void exportCode(quaternary a,int b){//aΪ���·�����������Ԫʽ��bΪ��ǰ�Ż��Ļ����飬cΪ����м������block�е�����
    char tmpStr[255]={'\0'};
	int Stack[255]={0};
	int StackTop=0,tmp;
	tmp=enqueue();//���ȣ����������м�ڵ�Ķ��С�
	while(tmp!=-1){
        Stack[StackTop++]=tmp;
        dag[tmp].isExported=1;
        tmp=dag[tmp].leftNum;
        while(tmp!=NULL){
            if((dag[tmp].isExported==1) || (dag[tmp].label==0) || (hasFather(tmp)==1))
                break;
            Stack[StackTop++]=tmp;
            dag[tmp].isExported=1;
            tmp=dag[tmp].leftNum ;
        }
        tmp=enqueue();
    }
	for(int i=0;i<nodeTop;i++)
		if(nodeTab[i].nodeName[0]=='$')
			for(int j=i+1;j<nodeTop;j++)
				if(nodeTab[i].nodeNumber==nodeTab[j].nodeNumber){
					strcpy(tmpStr,nodeTab[j].nodeName);//��¼�±��ĵ���ʱ��������
					strcpy(nodeTab[j].nodeName,nodeTab[i].nodeName);
					//Ȼ�󣬰ѵ�ǰ����������и�������к���������������漰����ʱ��������ȫ���������޸ġ�
					for(int k=0;k<blocks[b].codeNum;k++ ){
                        if(strcmp(blocks[b].code[k].var1,tmpStr)==0)
                            strcpy(blocks[b].code[k].var1,nodeTab[i].nodeName);
                        if(strcmp(blocks[b].code[k].var2,tmpStr)==0)
                            strcpy(blocks[b].code[k].var2,nodeTab[i].nodeName);
                        if(strcmp(blocks[b].code[k].var3,tmpStr)==0)
                            strcpy(blocks[b].code[k].var3,nodeTab[i].nodeName);
					}
				}
	for(int i=StackTop-1;i>=0;i--){
        strcpy(newcode[newcodelen].op,dag[Stack[i]].labelName);
		if(dag[Stack[i]].rightNum!=NULL){//op var1 var2 var3
			if(dag[dag[Stack[i]].leftNum].label==0)//���ӽڵ���Ҷ�ڵ�
				strcpy(newcode[newcodelen].var1,dag[dag[Stack[i]].leftNum].labelName);//Ҷ�ڵ�ֱ�ӿ�������.
			else
                strcpy(newcode[newcodelen].var1,getNodeName(dag[Stack[i]].leftNum));
			if(dag[dag[Stack[i]].rightNum].label==0)
				strcpy(newcode[newcodelen].var2,dag[dag[Stack[i]].rightNum].labelName);
			else
                strcpy(newcode[newcodelen].var2,getNodeName(dag[Stack[i]].rightNum));
		}
		else{//opp var1  xxx  var3
            if( dag[dag[Stack[i]].leftNum].label==0)//��ڵ���Ҷ�ڵ�
                strcpy(newcode[newcodelen].var1,dag[dag[Stack[i]].leftNum].labelName);
            else
                strcpy(newcode[newcodelen].var1,getNodeName(dag[Stack[i]].leftNum));
		}
        strcpy(newcode[newcodelen].var3,getNodeName(dag[Stack[i]].nodeNum));
		newcodelen++;
	}
	strcpy(newcode[newcodelen].op,a.op);
    strcpy(newcode[newcodelen].var1,a.var1);
    strcpy(newcode[newcodelen].var2,a.var2);
    strcpy(newcode[newcodelen++].var3,a.var3);
}

void delComExpr(){
    for(int j=0;j<blockNum;j++){
        clearDag();
        for(int i=0;i<blocks[j].codeNum;i++){
            if(strcmp(blocks[j].code[i].op,"add")==0 || strcmp(blocks[j].code[i].op,"sub")==0 ||
               strcmp(blocks[j].code[i].op,"imul")==0 || strcmp(blocks[j].code[i].op,"idiv")==0||
               strcmp(blocks[j].code[i].op,"[]=")==0 || strcmp(blocks[j].code[i].op,"=[]")==0)
                newNode(blocks[j].code[i],2);
            else if(strcmp(blocks[j].code[i].op,"mov")==0 && strcmp(blocks[j].code[i].var2," ")==0)
                newNode(blocks[j].code[i],1);
            else{
                exportCode(blocks[j].code[i],j);
                clearDag();
            }
        }
    }
}

int defined(block a,char s[]){
	if(strcmp(s,"")==0||strcmp(s," ")==0)
		return 1;
	for(int i=0;i<a.defNum;i++)
		if(strcmp(a.defVar[i],s)==0)
			return 1;
	return 0;
}

int used(block a,char s[]){
	if(strcmp(s,"")==0||strcmp(s," ")==0)
		return 1;
	for(int i=0;i<a.useNum;i++)
		if(strcmp(a.useVar[i],s)==0)
			return 1;
	return 0;
}

int searchIn(block a,char s[]){
	if(strcmp(s,"")==0||strcmp(s," ")==0)
		return 1;
	for(int i=0;i<a.inNum;i++)
		if( strcmp(a.inVar[i],s)==0)
			return 1;
	return 0;
}

int searchOut(block a,char s[]){
	if(strcmp(s,"")==0||strcmp(s," ")==0)
		return 1;
	for(int i=0;i<a.outNum;i++)
		if(strcmp(a.outVar[i],s)==0)
			return 1;
	return 0;
}

void addDefUse(int du,char s[],int bIndex){
    if(du==0){
        if(defined(blocks[bIndex],s)==0)
            if(used(blocks[bIndex],s)==0){
                strcpy(blocks[bIndex].useVar[blocks[bIndex].useNum],s);
                blocks[bIndex].useNum++;
            }
    }else{
        if(used(blocks[bIndex],s)==0)
            if(defined(blocks[bIndex],s)==0){
                strcpy(blocks[bIndex].defVar[blocks[bIndex].defNum],s);
                blocks[bIndex].defNum++;
            }
    }
}

void defAndUse(int d){
	for(int i=0;i<flow[d].blockNum;i++){//��һ��ѭ������ͼ�еĻ�����
	    int bIndex=flow[d].block[i];
		for(int j=0;j<blocks[bIndex].codeNum;j++){
			if(strcmp(blocks[bIndex].code[j].op,"add")==0||strcmp(blocks[bIndex].code[j].op,"sub")==0||
               strcmp(blocks[bIndex].code[j].op,"imul")==0||strcmp(blocks[bIndex].code[j].op,"idiv")==0||
               strcmp(blocks[bIndex].code[j].op,"[]=")==0||strcmp(blocks[bIndex].code[j].op,"=[]")==0){
                //op var1 var2 var3
                addDefUse(0,blocks[bIndex].code[j].var1,bIndex);
                addDefUse(0,blocks[bIndex].code[j].var2,bIndex);
                addDefUse(1,blocks[bIndex].code[j].var3,bIndex);
            }
			else if(strcmp(blocks[bIndex].code[j].op,"mov")==0){
                 //mov var1 ,   ,var3
                addDefUse(0,blocks[bIndex].code[j].var1,bIndex);
                addDefUse(1,blocks[bIndex].code[j].var3,bIndex);
			}
			else if(strcmp(blocks[bIndex].code[j].op,"jnge")==0||strcmp(blocks[bIndex].code[j].op,"jle")==0||
                    strcmp(blocks[bIndex].code[j].op,"jge")==0||strcmp(blocks[bIndex].code[j].op,"jnle")==0||
                    strcmp(blocks[bIndex].code[j].op,"jz")==0||strcmp(blocks[bIndex].code[j].op,"jnz")==0){
                    //sma var1 var2 lab
                addDefUse(0,blocks[bIndex].code[j].var1,bIndex);
                addDefUse(0,blocks[bIndex].code[j].var2,bIndex);
            }
            else if(strcmp(blocks[bIndex].code[j].op,"call")==0)
                addDefUse(1,blocks[bIndex].code[j].var3,bIndex);
            else if(strcmp(blocks[bIndex].code[j].op,"scanf")==0)
                addDefUse(0,blocks[bIndex].code[j].var3,bIndex);
            else if(strcmp(blocks[bIndex].code[j].op,"printf")==0)
                addDefUse(0,blocks[bIndex].code[j].var2,bIndex);
        }
	}
}

void inAndOut(int d){
	for(int i=0;i<flow[d].blockNum;i++){//���ѭ����ÿһ���������
	    int bIndex=flow[d].block[i];
        for(int k=0;k<2;k++)
            if(blocks[bIndex].nextBlock[k]!=NULL)
                for(int j=0;j<blocks[blocks[bIndex].nextBlock[k]].inNum;j++)
                    if(searchOut(blocks[bIndex],blocks[blocks[bIndex].nextBlock[k]].inVar[j])==0){//Out[b]=��̻�����in[p]
                        strcpy(blocks[bIndex].outVar[blocks[bIndex].outNum],blocks[blocks[bIndex].nextBlock[k]].inVar[j]);
                        blocks[bIndex].outNum++;
                    }
		for(int j=0;j<blocks[bIndex].useNum;j++)//in����=use[b]����out[b]-def[b]��
			if(searchIn(blocks[bIndex],blocks[bIndex].useVar[j])==0){
				strcpy(blocks[bIndex].inVar[blocks[bIndex].inNum],blocks[bIndex].useVar[j]);
				blocks[bIndex].inNum++;
			}
		for(int j=0;j<blocks[bIndex].outNum;j++)
			if(searchIn(blocks[bIndex],blocks[bIndex].outVar[j])==0){//��ÿһ����Outvar�����Ԫ��(�һ�δ�ŵ�in������)����ڲ���def����
				if(defined(blocks[bIndex],blocks[bIndex].outVar[j])==0){//�������def�������棬���������������뵽in������ȥ
                  strcpy(blocks[bIndex].inVar[blocks[bIndex].inNum],blocks[bIndex].outVar[j]);
                  blocks[bIndex].inNum++;
				}
			}
	}
}

void BuildConflict(char Var[],char s[]){
	for(int i=0;i<conTop;i++){
		if(strcmp(conTab[i].varName,Var)==0){
			strcpy(conTab[i].conflictVar[conTab[i].conNum],s);
			conTab[i].conNum++;
			return ;
		}
	}
}

//�жϱ�������ΪVAR�ı����Ƿ����ΪS�ı�����ͻ(Ҳ������ֹ�ظ���ӽ���Var�ĳ�ͻ��������)
int IsConflict(char Var[],char s[]){
	for(int i=0;i<conTop;i++)
		if(strcmp(conTab[i].varName,Var)==0)
			for(int j=0;j<conTab[i].conNum;j++)
				if(strcmp(conTab[i].conflictVar[j],s)==0)
					return 1;
	return 0;
}

void graph(int f){
	for(int i=0;i<flow[f].blockNum;i++){
        int b=flow[f].block[i];
		for(int j=0;j<blocks[b].inNum;j++){
		    int repeat=0;
            for(int k=0;k<conTop;k++)
		        if(strcmp(conTab[k].varName,blocks[b].inVar[j])==0)
			        repeat=1;
            if(repeat==0)//�Ƚ�ÿ����in��out�����ж����ֵı�������Խ������ı����������ͻ����
                if(searchOut(blocks[b],blocks[b].inVar[j]))
                    strcpy(conTab[conTop++].varName,blocks[b].inVar[j]);
            for(int k=0;k<blocks[b].inNum;k++)//��ͬһ��������ڴ�����Ծ�ı���֮����ڳ�ͻ
                if(strcmp(blocks[b].inVar[k],blocks[b].inVar[j])!=0)//��ͬһ��������ڴ�����Ծ�Ĳ�ͬ����
                   if(searchOut(blocks[b],blocks[b].inVar[k]))//��������ںͳ��ڴ�����Ծ�ı������ǿ�Խ������ı������Ż�ŵ���ͻ����ȥ
                    if(IsConflict(blocks[b].inVar[j],blocks[b].inVar[k])==0)//��δ������ͻ��ϵ
                        BuildConflict(blocks[b].inVar[j],blocks[b].inVar[k]);
		}
	}
}

int compare(block a[],int f){
	for(int i=0;i<flow[f].blockNum;i++){
		for(int j=0;j<blocks[flow[f].block[i]].inNum;j++)
			if(strcmp(blocks[flow[f].block[i]].inVar[j],a[i].inVar[j])!=0)
				return 0;
		for(int j=0;j<blocks[flow[f].block[i]].outNum;j++)
			if(strcmp(blocks[flow[f].block[i]].outVar[j],a[i].outVar[j])!=0)
				return 0;
	}
	return 1;
}

void cleanConflictGraphic(){//��ճ�ͻͼ
	int i,j;
	for(i=0;i<conTop;i++){
		conTab[i].isAllocted=0;
		conTab[i].Reg=NULL;
		conTab[i].tmpNum=0;
		strcpy(conTab[i].varName,"");
		for(j=0;j<conTab[i].conNum;j++)
		    strcpy(conTab[i].conflictVar[j],"");
		conTab[i].conNum=0;
	}
	conTop=0;
}

int nextConf(){
	for(int i=0;i<conTop;i++)//�ڳ�ͻͼ���ҵ���һ�����ӱ���ĿС��K�Ľڵ�
		if((conTab[i].tmpNum<2)&&(conTab[i].isAllocted!=1))
			return i;
	return -1;
}

void allocate(){
	int i,j,k;
	int tmp=conTop;//��ͻͼ��ʣ�µĽڵ���Ŀ
	int regsNumer=0;
	for(i=0;i<conTop;i++)
		conTab[i].tmpNum=conTab[i].conNum;
	while(tmp>0){
		i=nextConf();//�ҵ���ͻͼ�е�һ�����ӱ���С��K�Ľڵ�
		while(i!=-1){//����Ҳ���������while
			conTab[i].isAllocted=1;//�����Ѿ������˼Ĵ���
            conTab[i].Reg=regsNumer%2;
            regsNumer++;
			printf("%s : reg %d\n",conTab[i].varName,conTab[i].Reg);
			tmp--;
			for(j=0;j<conTop;j++)//������ڵ�ӳ�ͻͼ�����ߣ������޸������ͻ�Ľڵ�ı���
				for(k=0;k<conTab[j].conNum;k++)
					if(strcmp(conTab[j].conflictVar[k],conTab[i].varName)==0)
						conTab[j].tmpNum=conTab[j].tmpNum-1;
			i=nextConf();
		}
		if(tmp<=0)break;
		for(i=0;i<conTop;i++)//�Ҳ�����ʱ��ѡȡ��һ���ڵ�������ڵ���2����Ϊ������ȫ�ּĴ����ڵ㣬������
			if((conTab[i].tmpNum>=2)&&(conTab[i].isAllocted!=1))
                break;
        conTab[i].isAllocted=1;//�����Ѿ������˼Ĵ���
        conTab[i].Reg=-1;
        printf("%s : reg %d\n",conTab[i].varName,conTab[i].Reg);
        tmp--;
        for(j=0;j<conTop;j++)
            for(k=0;k<conTab[j].conNum;k++)
                if(strcmp(conTab[j].conflictVar[k],conTab[i].varName)==0)
                    conTab[j].tmpNum=conTab[j].tmpNum-1;
	}
}

void GlobalRegisterAllocation(){
    int iter=0;
	for(int i=0;i<flowTop;i++){
		defAndUse(i);//�ȷ���def��use
		while(iter<100){
			for(int j=0;j<flow[i].blockNum;j++){
				tmpB[tmpTop].inNum=blocks[flow[i].block[j]].inNum;
				tmpB[tmpTop].outNum=blocks[flow[i].block[j]].outNum;
				for(int k=0;k<blocks[flow[i].block[j]].inNum;k++)
					strcpy(tmpB[tmpTop].inVar[k],blocks[flow[i].block[j]].inVar[k]);
				for(int k=0;k<blocks[flow[i].block[j]].outNum;k++)
					strcpy(tmpB[tmpTop].outVar[k],blocks[flow[i].block[j]].outVar[k]);
				tmpTop++;
			}
			inAndOut(i);
			if(compare(tmpB,i)==1)break;
			for(int k=0;k<tmpTop;k++){
                tmpB[k].outNum=0;
                tmpB[k].inNum=0;
			}
			tmpTop=0;
			iter++;
		}
		iter=0;
		graph(i);
		allocate();
		cleanConflictGraphic();
	}
}

void printOptimize(){
    FILE *f=fopen("opticode.txt","w+");
	int i= 0;
	while (i<newcodelen) {
        fprintf(f,"%s,\t",newcode[i].op);
        fprintf(f,"%s,\t",newcode[i].var1);
        fprintf(f,"%s,\t",newcode[i].var2);
        fprintf(f,"%s;\n",newcode[i].var3);
		i++;
	}
	fclose(f);
}

void printFlow(){
    FILE *f=fopen("flow.txt","w+");
	for(int i=0;i<flowTop;i++){
        fprintf(f,"//////////flow%d//////////\n",i);
        for(int j=0;j<flow[i].blockNum;j++){
            fprintf(f,"/////block%d/////%d  %d\n",j,blocks[flow[i].block[j]].nextBlock[0],blocks[flow[i].block[j]].nextBlock[1]);
            for(int k=0;k<blocks[flow[i].block[j]].codeNum;k++){
                quaternary a=blocks[flow[i].block[j]].code[k];
                fprintf(f,"%s,\t",a.op);
                fprintf(f,"%s,\t",a.var1);
                fprintf(f,"%s,\t",a.var2);
                fprintf(f,"%s;\n",a.var3);
            }
        }
	}
	fclose(f);
}

void optimize(){
    constFold();
    markBlockHead();
    printOptimize();
    divideBlock();
    buildFlow();
    linkBlock();//����������֮�����ϵ
    printFlow();
    delComExpr();
    GlobalRegisterAllocation();
    printOptimize();
}
