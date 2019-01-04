#define START "START"
#define END "END"
#define BYTE "BYTE"
#define WORD "WORD"
#define RESB "RESB"
#define RESW "RESW"    
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
struct inst_with_loc{
	char label[20];
	char opcode[20];
	char operand[20];
	int loc;
};
struct pro_detail{
	char name[16];
	int start;
	int end;
};
struct symbole_list{
	char label[16];
	char loc[8];
};
struct opcode_list{
	char opcode[8];
	char num[2];
};
void gen_object_program(struct pro_detail pd);
struct pro_detail  generate_object_code(struct inst_with_loc *inst,int line_count);
struct inst_with_loc *generate_locations(FILE *code_fp,int *line_count);
void  main()
{
	char chk[2],file_name[20];
	FILE *code_fp;
	struct inst_with_loc *iwl;
	int line_count=0;
	while(1)
	{
		printf("\n--------Main Menu----------");
		printf("\n1) Enter File Name of the code");
		printf("\n2) Generate Location Of Instructions & Generate Symbole table");
		printf("\n3) Generate Object Code");
		printf("\n4) Exit");
		printf("\nPlease Select Your Option: ");
		scanf("%s",chk);
		switch(chk[0])
		{
			case '1':
				scanf("%s",file_name);
				code_fp=fopen(file_name,"r");
				if(code_fp==NULL)
				{
					printf("\n!!!File does't exist");
					continue;
				}
				else
				{
					printf("File Successfully Opened");
				}
				break;
			case '2':
				iwl=generate_locations(code_fp,&line_count);
				break;
			case '3':
				gen_object_program(generate_object_code(iwl,line_count));
				break;
			case '4':
				return;
				break;
			default:
				printf("Invalid selection: ");	
				break;
		}
	}
}
struct inst_with_loc *generate_locations(FILE *code_fp,int *line_count)
{
	char ch[64],label[16],opcode[16],operand[16],temp1[16];
	int count,i,j,loc,inc,k=0,state=1;
	FILE *temp,*loc_fp,*symbole_tab;
	static struct inst_with_loc inst_loc[128];
	loc_fp=fopen("inst_loc.txt","w");
	symbole_tab=fopen("symbole_table.txt","w");
	while(fgets(ch,64,code_fp)!=NULL)
	{
		count=strlen(ch);
		//printf("zadu: %s",ch);
		strcpy(opcode,"\0");
		strcpy(label,"\0");
		strcpy(operand,"\0");
		if(strlen(ch)<=2)continue;
		if(ch[0]=='\n' || ch[0]=='.') continue;	
		for(i=0,j=0;i<count;i++)
		{
			if(ch[i]=='\t' || ch[i]=='\n' || ch[i]==' ')
			{
				temp1[j]='\0';
				j=0;
				if(strlen(temp1)>0)
				{
					if(state==1)
					{
						strcpy(label,temp1);
						state=2;
					}
					else if(state==2)
					{
						strcpy(opcode,temp1);
						state=3;
					}
					else if(state==3)
					{
						strcpy(operand,temp1);
						state=1;
					}
				}
			}
			else
			{
				temp1[j]=ch[i];
				j++;
			}
		}
		if(state==2)
		{
			strcpy(opcode,label);
			strcpy(label,"\0");
			strcpy(operand,"\0"); 
		}		
		if(state==3)
		{
			strcpy(operand,opcode);
			strcpy(opcode,label);
			strcpy(label,"\0"); 
		}	
		state=1;     
		if(strcmp(opcode,"START")==0)
		{
			temp=fopen("temp.txt","w");
			fprintf(temp,"%s",operand);
			fclose(temp);

			temp=fopen("temp.txt","r");
			fscanf(temp,"%x",&loc);
			fclose(temp);
			fprintf(loc_fp,"%x\t%s\t%s\t%s\n",loc,label,opcode,operand);

			inst_loc[k].loc=loc;
			strcpy(inst_loc[k].label,label);
			strcpy(inst_loc[k].opcode,opcode);
			strcpy(inst_loc[k].operand,operand);
			inst_loc[k].operand[strlen(inst_loc[k].operand)]='\0';
			k++;
			fprintf(symbole_tab,"%s\t%x\n",label,loc);
			continue;
		}
		fprintf(loc_fp,"%x\t%s\t%s\t%s\n",loc,label,opcode,operand);
		inst_loc[k].loc=loc;
		strcpy(inst_loc[k].label,label);
		strcpy(inst_loc[k].opcode,opcode);
		strcpy(inst_loc[k].operand,operand);

		k++;
		if(strlen(label)!=0)
			fprintf(symbole_tab,"%s\t%x\n",label,loc);
		if(strcmp(opcode,BYTE)==0 || strcmp(opcode,RESW)==0 || strcmp(opcode,RESB)==0 )
		{
			if(strcmp(opcode,BYTE)==0)
			{
				inc=strlen(operand)-4;
			}
			else if(strcmp(opcode,RESW)==0)
			{
				inc=3*(atoi(operand));
			}
			else
			{
				inc=atoi(operand);
			}
			loc=loc+inc;		
		}
		else
		{
			loc=loc+3;	
		}
	}
	fclose(symbole_tab);
	fclose(loc_fp);
	*line_count=k;
	return(inst_loc);
}
struct pro_detail  generate_object_code(struct inst_with_loc *inst_loc,int line_count)
{
	int i=0,j=0,k,op_list_count,sy_list_count=0,temp,start=0x0,end=0x0;
	struct inst_with_loc inst;
	struct symbole_list sl[64];
	struct opcode_list ol[64];
	struct pro_detail pd;
	char object_code[8],operand_temp[16],pro_name[16];
	int object_code1=0x0,temp1=0x0;
	FILE *op_list,*fp,*object_program;
	char buff[8];
	op_list=fopen("opcode_table.txt","r");
	object_program=fopen("object_code.txt","w");
	fp=fopen("with_object.txt","w");

	if(op_list==NULL)
	{
		printf("\nanable to open file");
	}
	else
	{
		while(fscanf(op_list,"%s\t%s\n",ol[i].opcode,ol[i].num)!=EOF)
		{
			i++;
		}
		op_list_count=i;
	}
	fclose(op_list);
	i=0;
	op_list=fopen("symbole_table.txt","r");

	if(op_list==NULL)
	{
		printf("\nanable to open file");
	}
	else
	{
		while(fscanf(op_list,"%s\t%s\n",sl[i].label,sl[i].loc)!=EOF)
		{
			i++;
		}
		sy_list_count=i;
	}
	fclose(op_list);
	for(i=0;i<line_count;i++)
	{
		inst=*(inst_loc+i);
		strcpy(object_code,"000000");
		if(strcmp(inst.opcode,START)==0 || strcmp(inst.opcode,RESW)==0 || strcmp(inst.opcode,RESB)==0 || strcmp(inst.opcode,END)==0)
		{
			if(strcmp(inst.opcode,START)==0)
			{
				strcpy(pro_name,inst.label);
				start=inst.loc;
			}
			if(strcmp(inst.opcode,END)==0)
			{
				end=inst.loc;
			}
			strcpy(object_code,"\0");
		}
		else if(strcmp(inst.opcode,BYTE)==0)
		{
			if(inst.operand[0]=='X' || inst.operand[0]=='x')
			{
				for(j=0;j+3<strlen(inst.operand);j++)
				{

					object_code[j]=inst.operand[j+2];
				}   
				object_code[j]='\0';
			}
			else if(inst.operand[0]=='c' || inst.operand[0]=='C')
			{
				for(j=0;j+3<strlen(inst.operand);j++)
				{
					temp1=0x0;
					temp1=temp1+(int)inst.operand[j+2];
					object_code1=object_code1* 0x100 + temp1;
				}   
				printf("%-15x%-15s%-15s%-15s%-15x\n",inst.loc,inst.label,inst.opcode,inst.operand,object_code1); 
				fprintf(fp,"%-15x%-15s%-15s%-15s%-15x\n",inst.loc,inst.label,inst.opcode,inst.operand,object_code1);
				fprintf(object_program,"%x %x\n",inst.loc,object_code1);
				continue;	
			}
			else
			{
				strcpy(object_code,"REMAIN");
			}
		}
		else if(strcmp(inst.opcode,WORD)==0)
		{
			temp=atoi(inst.operand);
			op_list=fopen("temp.txt","w");
			fprintf(op_list,"%x",temp);
			fclose(op_list);
			op_list=fopen("temp.txt","r");
			fscanf(op_list,"%s",buff);
			fclose(op_list);
			for(k=0;k<strlen(buff);k++)
			{
				object_code[5-k]=buff[strlen(buff)-(k+1)];
			}
		}
		else if(inst.operand[strlen(inst.operand)-2]==',' && (inst.operand[strlen(inst.operand)-1]=='x' || inst.operand[strlen(inst.operand)-1]=='X'))
		{
			for(k=0;k<op_list_count;k++)
			{
				if(strcmp(inst.opcode,ol[k].opcode)==0)
				{
					object_code[0]=ol[k].num[0];
					object_code[1]=ol[k].num[1];
					break;
				}
			}		
			strcpy(operand_temp,inst.operand);
			operand_temp[strlen(inst.operand)-2]='\0';
			for(k=0;k<sy_list_count;k++)
			{
				if(strcmp(operand_temp,sl[k].label)==0)
				{
					object_code[2]=sl[k].loc[0];
					object_code[3]=sl[k].loc[1];
					object_code[4]=sl[k].loc[2];
					object_code[5]=sl[k].loc[3];
					break;
				}
			}
			if(object_code[2]=='0')
				object_code[2]='8';
			else if(object_code[2]=='1')
				object_code[2]='9';
			else if(object_code[2]=='2')
				object_code[2]='a';
			else if(object_code[2]=='3')
				object_code[2]='b';
			else if(object_code[2]=='4')
				object_code[2]='c';
			else if(object_code[2]=='5')
				object_code[2]='d';
			else if(object_code[2]=='6')
				object_code[2]='e';
			else if(object_code[2]=='7')
				object_code[2]='f';
			else
			{

			}
		}
		else
		{
			for(k=0;k<op_list_count;k++)
			{
				if(strcmp(inst.opcode,ol[k].opcode)==0)
				{
					object_code[0]=ol[k].num[0];
					object_code[1]=ol[k].num[1];
					break;
				}
			}		
			for(k=0;k<sy_list_count;k++)
			{
				if(strcmp(inst.operand,sl[k].label)==0)
				{
					object_code[2]=sl[k].loc[0];
					object_code[3]=sl[k].loc[1];
					object_code[4]=sl[k].loc[2];
					object_code[5]=sl[k].loc[3];
					break;
				}
			}
			printf("%-15x%-15s%-15s%-15s%-15s\n",inst.loc,inst.label,inst.opcode,inst.operand,object_code); 
			fprintf(fp,"%-15x%-15s%-15s%-15s%-15s\n",inst.loc,inst.label,inst.opcode,inst.operand,object_code); 
			if(strlen(object_code)>0)
				fprintf(object_program,"%x %s\n",inst.loc,object_code);
			continue;
		}
		if(strcmp(inst.opcode,END)==0) 	
		{
			printf("%-15s%-15s%-15s%-15s%-15s\n",inst.label,inst.label,inst.opcode,inst.operand,object_code); 
			fprintf(fp,"%-15s%-15s%-15s%-15s%-15s\n",inst.label,inst.label,inst.opcode,inst.operand,object_code); 
			continue;
		}
		printf("%-15x%-15s%-15s%-15s%-15s\n",inst.loc,inst.label,inst.opcode,inst.operand,object_code); 
		fprintf(fp,"%-15x%-15s%-15s%-15s%-15s\n",inst.loc,inst.label,inst.opcode,inst.operand,object_code); 
		if(strlen(object_code)>0)
			fprintf(object_program,"%x %s\n",inst.loc,object_code);
	}
	fclose(fp);	
	fclose(object_program);
	strcpy(pd.name,pro_name);
	pd.end=end;
	pd.start=start;
	return(pd);
}

void gen_object_program(struct pro_detail pd)
{

	FILE *object_code,*object_program;
	int loc,fp_point=0,prev=pd.start,count=0x0;
	char op[8];
	float fc;
	object_code=fopen("object_code.txt","r");
	object_program=fopen("object_program.txt","w");
	fprintf(object_program,"H^%-6s^%06x^%06x",pd.name,pd.start,pd.end-pd.start);
	fprintf(object_program,"\nT^%06x^00",pd.start);
	fp_point=ftell(object_program);

	while(fscanf(object_code,"%x %s\n",&loc,op)!=EOF)
	{
		if(loc-prev>100 || count+strlen(op)>60)
		{

			if(count<16)
				fseek(object_program,fp_point-1,SEEK_SET);
			else
				fseek(object_program,fp_point-2,SEEK_SET);
			fc=count/2;
			if(fc-count/2>0)
				count=(count/2)+1;
			else
				count=count/2;
			fprintf(object_program,"%x",count); 
			fseek(object_program,0,SEEK_END);
			fprintf(object_program,"\nT^%06x^00",loc);
			fp_point=ftell(object_program);
			count=0x0;
			fprintf(object_program,"^%s",op);
			count=count+(int)strlen(op);
		}
		else
		{
			fprintf(object_program,"^%s",op);
			count=count+(int)strlen(op);
		}
		prev=loc;	

	}
	if(count<16)
		fseek(object_program,fp_point-1,SEEK_SET);
	else
		fseek(object_program,fp_point-2,SEEK_SET);
	fc=count/2;
	if(fc-count/2>0)
		count=(count/2)+1;
	else
		count=count/2;
	fprintf(object_program,"%x",count); 
	fseek(object_program,0,SEEK_END);

	fprintf(object_program,"\nE^%06x\n",pd.start);


	fclose(object_code);
	fclose(object_program);

}
