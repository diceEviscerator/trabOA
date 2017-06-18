#include <stdio.h>
#include "drive_lib.h"

track_array *cylinder;
fatlist_s *fatlist_sectors;
fatent_s *fatlist_files_initial=NULL;
int num_cil=3; //Contador do numero de cilindros

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
  cylinder = (track_array*)calloc(512*60*5*num_cil,sizeof(block)); //Alocando 1 cilindros
  //num_cil++;
  fatlist_sectors = (fatlist*)calloc(512*60*5*num_cil,sizeof(fatlist));
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
    cylinder[c].track[t].sector[s].bytes_s[b]='0';
    b++;
  }
}

void write_file () {
  FILE *file;
  char file_name[100], c;
  int t=0, s=0, i=0, j=0, fat_sector=0, fat_sector_search=0, cont=0, fat_sector_search_aux=0;
  const float time_ltnc=6, time_seek=4, time_trns=0.000390652;
  float time_total=0;
  fatent_s *fatlist_files_actual, *fatlist_files_new;
  bool flag_inicializacao = false;

  printf("Informe o nome do arquivo, com '.txt': ");
  scanf("%s", file_name);
  file=fopen(file_name, "r");
  if(file != NULL) {
    printf("Arquivo aberto com sucesso.\n");
    time_total=time_total+time_seek;
  } else {
    printf("Arquivo inexistente!\n");
    return;
  }

  while(fatlist_sectors[s].used != 0) { //Procurando pelo primeiro cluster vazio
    s += 4; //Numero de setores aumenta de 4 em 4
    if(s%60==0) {
      j++; //Indo para o cilindro de numero j
      s = 300*j;
      if(j == num_cil) {
        t++; //Indo para a trilha numero t
        j=0;
        s = 60*t; 
      }
    }
  }
  fat_sector = s;

  fatlist_files_actual=(fatent_s*)malloc(sizeof(fatent_s));
  if (fatlist_files_initial==NULL){
    fatlist_files_initial=(fatent_s*)calloc(1,sizeof(fatent_s));
    flag_inicializacao = true;
  } else {/*se a lista fat já está inicializada*/
    fatlist_files_actual=fatlist_files_initial;
    while(fatlist_files_actual->next_file != NULL) {/*move atual até o fim da lista*/
      fatlist_files_actual=fatlist_files_actual->next_file;
    }
  }

  fatlist_files_new=(fatent_s*)calloc(1,sizeof(fatent_s)); //Definindo novo arquivo
  strcpy(fatlist_files_new->file_name, file_name);
  fatlist_files_new->first_sector=fat_sector;
  fatlist_files_new->next_file = NULL;

  if(flag_inicializacao == true) { //Se for o primeiro arquivo
    strcpy(fatlist_files_initial->file_name,fatlist_files_new->file_name);
    fatlist_files_initial->first_sector = fatlist_files_new->first_sector;
    fatlist_files_initial->next_file = fatlist_files_new->next_file;
  } else {
    fatlist_files_actual->next_file = fatlist_files_new;
  }
  fat_sector_search = fat_sector;
  fat_sector_search_aux = fat_sector_search;
  do {
    fread(&c,sizeof(char),1,file);
    if(feof(file)){
      cylinder[j].track[t].sector[s].bytes_s[i]='\0';
      break;
    }
    //printf("fat_sector_search: %d\n", fat_sector_search);
    fatlist_sectors[fat_sector_search].used=1; //Marcando setores como usados
    cylinder[j].track[t].sector[s].bytes_s[i]=c; //Gravando caracter no byte i
    time_total=time_total+time_trns;
    i++;

    if(fat_sector_search < fat_sector_search_aux+3) { //Se nao for o ultimo setor do cluster
      fatlist_sectors[fat_sector_search].next = fat_sector_search+1;
    }

    if(i==512) { //Se chegou ao final do setor
      time_total=time_total+time_ltnc;
      s++;
      fat_sector_search++;
      i=0;
      if(fat_sector_search == fat_sector_search_aux+4) { //Se saimos do mesmo cluster
        fat_sector_search_aux = fat_sector_search-1; //Salvando numero do ultimo setor do ultimo cluster utilizado
        t++;
        fat_sector_search = fat_sector+60; //Proximo cluster do arquivo deve ser gravado na proxima trilha, no mesmo cilindro (se disponivel)
        fat_sector += 60;
        while(fatlist_sectors[fat_sector_search].used != 0) {
          t++;
          fat_sector_search = fat_sector_search+60; //Gravando proximo cluster do arquivo na proxima trilha, no mesmo cilindro
          if(t == 5 && j != num_cil) { //Se chegou a ultima trilha e nao foi do utlimo cilindro
            t = 0;
            j++;
            time_total=time_total+time_seek;
          }
          if(j == num_cil && t == 5) { //Se chegamos ao final do ultimo cilindro, alocar mais espaco
            printf("Disco cheio \n");
            return;
            cylinder = (track_array*)calloc(512*60*5,sizeof(block)); //Alocando 1 cilindro
            //num_cil++;
            fatlist_sectors = (fatlist*)calloc(512*60*5,sizeof(fatlist)); //Alocando mais setores*/
          }
        }
        s = fat_sector_search;
        fatlist_sectors[fat_sector_search_aux].next = fat_sector_search;
        fat_sector_search_aux = fat_sector_search;
       }
    }
  } while(c != EOF);
  

  fatlist_sectors[fat_sector_search].eof=1;
  fatlist_sectors[fat_sector_search].next=0; //Se for o ultimo setor de um arquivo, apontar para o inicial
  cont = fat_sector_search;
  fat_sector_search_aux = 0;
  if (cont == 0){
    cont = 4;
  }
  while((fat_sector_search_aux < cont)) { //Se o ultimo setor utilizado nao foi o ultimo de um cluster, marcar outros setores do cluster
    if(fat_sector_search_aux == 0) {
      fat_sector_search_aux += 3;
    } else {
      fat_sector_search_aux += 4;
    }
  }
  cont = fat_sector_search_aux - cont;

  if(cont != 0) {
    fatlist_sectors[fat_sector_search].next = fat_sector_search+1;
  } else {
    fatlist_sectors[fat_sector_search].next = fat_sector_search;
  }
  for(s=1;s<=cont;s++) {
    fatlist_sectors[fat_sector_search+s].used = 1;
    if(s != cont) {
      fatlist_sectors[fat_sector_search+s].next = fat_sector_search+s+1;
    } else {
      fatlist_sectors[fat_sector_search+s].next = fat_sector_search+cont; //Se eh o ultimo setor do ultimo cluster, apontar pra ele mesmo
    }
  }
  fclose(file);
  printf("Tempo total de acesso ao disco: %fms\n", time_total);
  return;
}

void read_file () {
  int sector_number=0, base_number=0, j=0, t=0, s=0, i=0, cont, comp=1;
  char file_name[100], c=0;
  fatent_s *fatlist_files_actual=NULL;
  FILE *out;

  out=fopen("saida.txt", "w");
  printf("Informe o nome do arquivo, com '.txt': ");
  scanf("%s", file_name);
  fatlist_files_actual=fatlist_files_initial;
  comp = strcmp((fatlist_files_actual->file_name), file_name);
  while ((comp!=0) && (fatlist_files_actual->next_file!=NULL)){ 
    fatlist_files_actual=fatlist_files_actual->next_file;
    comp = strcmp((fatlist_files_actual->file_name), file_name);
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
    base_number=sector_number;
    j=base_number/300;
    base_number=base_number%300;
    t=base_number/60;
    base_number=base_number%60;
    s=base_number;
    i=0;
    while(i<512){
        c=cylinder[j].track[t].sector[s].bytes_s[i];
        if (c=='\0'){
          break;
        }
        fprintf(out, "%c", c);
        i++;
    }
  }while((fatlist_sectors[sector_number].eof)!=1);
  fclose(out);
  printf("Arquivo lido com sucesso. \n");
  return;
}

void delete_file() { //Necessario alterar
  char file_name[100];
  fatent_s *fatlist_files_actual, *fatlist_files_auxiliar;
  unsigned int sector;
  bool primeiro_arquivo = false;

  printf("Digite o nome do arquivo: "); //Pegando nome do arquivo a ser excluido
  scanf("%s", file_name);
  if(fatlist_files_initial != NULL) { //Se a tabela FAT nao estiver vazia
    fatlist_files_actual = fatlist_files_initial; 
    while(fatlist_files_actual != NULL) { //Enquanto nao chegarmos no ultimo arquivo da tabela FAT
      if(!strcmp(fatlist_files_actual->file_name, file_name)) { //Se o arquivo pertencer a tabela FAT
          sector = fatlist_files_actual->first_sector; //Pegando primeiro setor do arquivo
          if(fatlist_files_actual->next_file != NULL) {
            fatlist_files_auxiliar = fatlist_files_actual->next_file; //Excluindo arquivo da tabela FAT
            strcpy(fatlist_files_actual->file_name, fatlist_files_auxiliar->file_name);
            fatlist_files_actual->first_sector = fatlist_files_auxiliar->first_sector;
            fatlist_files_actual->next_file = fatlist_files_auxiliar->next_file; //Fim da exclusao
            break;
          } else {
            primeiro_arquivo = true; //Se chegou aqui, eh o primeiro arquivo
            if(fatlist_files_initial->next_file == NULL) {
              fatlist_files_initial = NULL; 
              break; 
            } else {
              fatlist_files_auxiliar->next_file = NULL;
              break;
            }
          }
      }
      if(fatlist_files_actual != NULL) {
        fatlist_files_auxiliar = fatlist_files_actual;
        fatlist_files_actual = fatlist_files_actual->next_file;
      }
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
    fatlist_sectors[sector].used = 0; //Marcando campo used com 0
    sector = fatlist_sectors[sector].next; //Indo para o proximo setor
  } while(fatlist_sectors[sector].next != sector);
  printf("Arquivo apagado com sucesso.\n");
}

void show_fat (){
  unsigned int sector_number=0;
  int cont=0;
  int total_sectors=0, sectors[200];
  fatent_s *fatlist_files_auxiliar = NULL;
  printf ("NOME \t\t TAMANHO EM DISCO \t LOCALIZACAO \n");

  if(fatlist_files_initial != NULL) {
    fatlist_files_auxiliar = fatlist_files_initial;
    while(fatlist_files_auxiliar != NULL) {
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
        } while((fatlist_sectors[sector_number].next) != sector_number);
        printf ("%s \t %d \t\t\t %u", fatlist_files_auxiliar->file_name, total_sectors*512, sectors[0]);
        cont=1;
        while(cont<total_sectors){
          printf (", %u", sectors[cont]);
            cont++;
        }
        printf("\n");
        fatlist_files_auxiliar = fatlist_files_auxiliar->next_file;
      }
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