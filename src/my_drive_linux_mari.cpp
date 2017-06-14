#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initialize_disk();

void show_menu ();

void menu_selection (int menu_option);

void write_file();

void read_file ();

void delete_file ();

void find_drive_sector(int fat_number, int j, int t, int s);

int find_fat_sector(int j, int t, int s);

void show_fat();

typedef struct block {
  char bytes_s[512];
} block;

typedef struct  sector_array {
  block sector[60];
} sector_array;

typedef struct track_array {
  sector_array track[5];
} track_array;

typedef struct  fatlist_s {
  unsigned int used;
  unsigned int eof;
  unsigned int next;
} fatlist;

typedef struct fatent_s {
  char file_name[100];
  unsigned int first_sector;
  struct fatent_s *next_file;
} fatent;
track_array *cylinder;
fatlist_s *fatlist_sectors;
fatent_s *fatlist_files_initial=NULL;
int num_cil=0; //Contador do numero de cilindros
void show_menu () { // simples função que limpa a tela e mostra o menu.
  //system("clear");
  printf("1 - Escrever Arquivo\n2 - Ler Arquivo\n3 - Apagar Arquivo\n4 - Mostrar Tabela FAT\n5 - Sair\n");
  return;
}

void menu_selection (int menu_option) { // função que direciona o programa para as funções dependendo da opção escolhida.
  switch (menu_option){
    case 1:
      system("clear");
      write_file();
      break;
    case 2:
      system("clear");
      read_file();
      break;
    case 3:
      system("clear");
      delete_file();	
      break;
    case 4:
      system("clear");
      show_fat();
      break;
    case 5:
      break;
    default:
      break;
  }
  return;
}

int find_fat_sector(int j, int t, int s){
  int sum;
  sum=(j*300)+(t*60)+s;
  return sum;
}
void find_drive_sector(int fat_number, int j, int t, int s){ // NECESSÁRIO CHECAR!!!!
  j=fat_number/300;
  fat_number=fat_number%300;
  t=fat_number/60;
  fat_number=fat_number%60;
  s=fat_number;
  return;
}
void initialize_disk(){
  int c=0, t=0, s=0, b=0;
  cylinder = (track_array*)calloc(512*60*5,sizeof(block));
  num_cil++;
  fatlist_sectors = (fatlist*)calloc(512*60*5,sizeof(fatlist));
  while(c<11){
    if(b==512){
      b=0;
      s++;
      if(s==60){
        s=0;
        t++;
        if(t==5){
          t=0;
          c++;
          if(c==10){
            return;
          }
        }
      }
    }
    cylinder[c].track[t].sector[s].bytes_s[b]=0;
    b++;
    //printf("%d %d %d %d, %c\n",c, t, s ,b, cylinder[c].track[t].sector[s].bytes_s[b]);
  }
}

void write_file (){
  FILE *file;
  char file_name[100], c;
  int t=0, s=0, i=0, j=0, fat_sector=0, fat_sector_search;
  fatent_s *fatlist_files_actual, *fatlist_files_new;
  bool flag_inicializacao = false, full_flag = false;
  //do{
    printf("Informe o nome do arquivo, com '.txt': ");
    scanf("%s", file_name);
    file=fopen(file_name, "r");
  //}while(file==NULL);
  printf("Arquivo aberto com sucesso.\n");
  //getchar();
  /*while(cylinder[j].track[t].sector[s*4].bytes_s[0]!=0){ 
    //getchar();
    s++;
    if(s==15){
      j++;
      s=0;
      if(j==10){
        t++;
        j=0;
        if(t==5){
          printf("Disco cheio\n");
          return;
        }
      }
    }
    printf("%d, %d, %d\n", s, j, t);
  }*/
  //fat_sector=find_fat_sector(j, t, s);
  while(fatlist_sectors[fat_sector].used==1){
    fat_sector++;
    if(fat_sector == (num_cil*512*60*5)-1) {
        full_flag = true;
        printf("Disco cheio\n");
        break;
    }
  }

  if(full_flag == true)
    return;

  fatlist_files_actual=(fatent_s*)malloc(sizeof(fatlist_files_actual));
  if (fatlist_files_initial==NULL){
    fatlist_files_initial=(fatent_s*)malloc(sizeof(fatlist_files_initial));
    //fatlist_files_actual=fatlist_files_initial;
    flag_inicializacao = true;
  }
  else{/*se a lista fat já está inicializada*/
    fatlist_files_actual=fatlist_files_initial;
    while(fatlist_files_actual->next_file != NULL) {/*move atual até o fim da lista*/
      fatlist_files_actual=fatlist_files_actual->next_file;
    }
  }
  fatlist_files_new=(fatent_s*)malloc(sizeof(fatlist_files_new));
  strcpy(fatlist_files_new->file_name, file_name);
  fatlist_files_new->first_sector=fat_sector;
  fatlist_files_new->next_file = NULL;
  if(flag_inicializacao == true) { //Se for o primeiro arquivo
    fatlist_files_initial = fatlist_files_new;
  } else {
    fatlist_files_actual->next_file=fatlist_files_new;
  }
  fat_sector_search=fat_sector;
  do{
    fread(&c,sizeof(char),1,file);
    if(feof(file)){break;}
    cylinder[j].track[t].sector[s].bytes_s[i]=c;
    if(fatlist_sectors[fat_sector_search].used==0){fatlist_sectors[fat_sector_search].used=1;}
    i++;
    if(i==512){
      s++;
      i=0;
      fatlist_sectors[fat_sector_search].next = fat_sector_search+1;
      fat_sector_search++;
    }
    if(s==60) {
      j++;
      s=0;
    }
    if(j==10) {
      t++;
      j=0;
    }
    if(t==5){
      printf("Disco cheio, dados truncados.\n");
      return;
    }
  }while(c != EOF);
  //while(fatlist_sectors[fat_sector_search].used==1){
    //fat_sector_search++;
  //}
  fatlist_sectors[fat_sector_search].eof=1;
  fclose(file);
  return;
}

void read_file () {
  int sector_number=0, j=0, t=0, s=0, i=0, cont, comp=1;
  char file_name[100], c=0;
  fatent_s *fatlist_files_actual=NULL;
  FILE *out;

  out=fopen("saida.txt", "w");
  printf("Informe o nome do arquivo, com '.txt': ");
  scanf("%s", file_name);
  fatlist_files_actual=fatlist_files_initial;
  comp = strcmp((fatlist_files_actual->file_name), file_name);
  while (((strcmp((fatlist_files_actual->file_name), file_name))!=0)&&(fatlist_files_actual->next_file!=NULL)){ 
    fatlist_files_actual=fatlist_files_actual->next_file;
  }
  if (comp != 0){
    printf("Arquivo não encontrado. \n");
    return;
  }
  do{
    if(cont==0){
      sector_number=fatlist_files_actual->first_sector;
      cont=1;
    }
    else{
      sector_number = fatlist_sectors[sector_number].next;
    }
    find_drive_sector(sector_number, j, t, s);
    for(i=0; i<512; i++){
      c=cylinder[j].track[t].sector[s].bytes_s[i];
      fprintf(out, "%c", c);
    }
    printf("Proximo Setor \n");
  }while((fatlist_sectors[sector_number].eof)!=1);
  fclose(out);
  printf("Arquivo lido com sucesso. \n");
  return;
}

void delete_file() {
  char file_name[100];
  fatent_s *fatlist_files_actual, *fatlist_files_auxiliar;
  unsigned int first_sector;
  bool primeiro_arquivo = false;

  printf("Digite o nome do arquivo: "); //Pegando nome do arquivo a ser excluido
  scanf("%s", file_name);
  //system("clear")
  if(fatlist_files_initial != NULL) { //Se a tabela FAT nao estiver vazia
    fatlist_files_actual = fatlist_files_initial; 
    while(fatlist_files_actual != NULL) { //Enquanto nao chegarmos no ultimo arquivo da tabela FAT
      if(!strcmp(fatlist_files_actual->file_name, file_name)) { //Se o arquivo pertencer a tabela FAT
          first_sector = fatlist_files_actual->first_sector; //Pegando primeiro setor do arquivo
          if(fatlist_files_actual->next_file != NULL) {
            fatlist_files_auxiliar = fatlist_files_actual->next_file; //Excluindo arquivo da tabela FAT
            strcpy(fatlist_files_actual->file_name, fatlist_files_auxiliar->file_name);
            fatlist_files_actual->first_sector = fatlist_files_auxiliar->first_sector;
            fatlist_files_actual->next_file = fatlist_files_auxiliar->next_file; //Fim da exclusao
          } else {
            primeiro_arquivo = true; //Se chegou aqui, eh o primeiro arquivo
            fatlist_files_initial = NULL;  
          }
      }
      if(fatlist_files_actual != NULL)
        fatlist_files_actual = fatlist_files_actual->next_file;
    } 
    if(fatlist_files_actual == NULL && primeiro_arquivo == false) {
      printf("Arquivo nao esta na tabela FAT.\n");
      return;
    }

  } else { //Se a tabela FAT estiver vazia
    printf("Tabela FAT vazia.\n");
    return;
  }
  do { //Enquanto nao chegarmos no ultimo setor do arquivo
    fatlist_sectors[first_sector].used = 0; //Marcando campo used com 0
    first_sector = fatlist_sectors[first_sector].next; //Indo para o proximo setor
  } while(fatlist_sectors[first_sector].eof != 1);
  printf("Arquivo apagado com sucesso.\n");
}

void show_fat (){
	int sector_number=0, j=0, t=0, s=0, cont=0;
	int total_sectors=0, sectors[200];
	fatent *fatlist_files_auxiliar = NULL;
	fatlist_files_auxiliar = fatlist_files_initial;

  printf ("NOME \t\t TAMANHO EM DISCO \t LOCALIZACAO \n");
	while(fatlist_files_auxiliar != NULL)
	{
    cont=0;
		sector_number=fatlist_files_auxiliar->first_sector;
		total_sectors=0;
		do{
  		if(cont==0){
        sector_number=fatlist_files_auxiliar->first_sector;
        cont=1;
      }
      else{
        sector_number = fatlist_sectors[sector_number].next;
      }
    	sectors[total_sectors]=sector_number;
      total_sectors++;
    } while((fatlist_sectors[sector_number].eof)!=1);
    printf ("%s \t %d \t\t\t %u", fatlist_files_auxiliar->file_name, total_sectors*512, sectors[0]);
    cont=1;
    while(cont<total_sectors){
    	printf (", %u", sectors[cont]);
      cont++;
    }
    printf("\n");
    fatlist_files_auxiliar = fatlist_files_auxiliar->next_file;
  }
  printf("\n\n");
}

int main (){
  int menu_option;
  initialize_disk();
  while (menu_option!=5){
    printf("\n");
    show_menu();
    printf("\nEscolha sua opcao: ");
    scanf("%d", &menu_option);
    while (menu_option!=1 && menu_option!=2 && menu_option!=3 && menu_option!=4 && menu_option!=5) { // garantia de entrada correta.
        printf("Escolha invalida!\n\n");
        show_menu();
        printf("\nEntre novamente: ");
        scanf("%d", &menu_option);
    }
    menu_selection(menu_option);
  }
  free(cylinder);
  free(fatlist_files_initial);
  free(fatlist_sectors);
  return 0;
}