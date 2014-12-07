#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#define NUMTHREADS 10
int controle = 255;

//DEFINE UMA ESTRUTURA PARA ARMAZENAR OS CODIGOS DAS CORES, O QUE SERA UTILIZADO COMO UM TIPO DE HASH;
struct codigo
{
    long long peso;
    char* cod;
};

typedef struct Celula *Apontador;

//CELULA É QUALQUER NÓ DA ARVORE OU QUALQUER ELEMENTO DA FILA
struct Celula
{
    int cor; //informação
    long long peso;
    char* ficheiro;
    Apontador prox; //ligação
    Apontador fesq;
    Apontador fdir;
};

//DECLARACAO DE UMA ESTRUTURA DE FILA
struct TipoFila
{
    Apontador Frente;
    Apontador Final;
    int Tamanho;
};

//DECLARACAO DE UMA ESTRUTURA DO TIPO ARVORE
struct TipoArvore
{
    Apontador Raiz;
};

int maior = 0;

void FazFilaVazia(struct TipoFila *Fila)
{
    Fila->Frente = (Apontador) malloc(sizeof(struct Celula));
    Fila->Final = Fila->Frente;
    Fila->Frente->prox = NULL;
    Fila->Tamanho = 0;
}

//INSERE OS ELEMENTOS NUMA FILA DE FORMA QUE A FILA SEMPRE ESTEJA ORDENADA DE FORMA CRESCENTE
void Insere(Apontador celula, struct TipoFila *Fila)
{
    if(Fila->Tamanho == 0)
    {
        Fila->Frente->prox = celula;
        Fila->Final = Fila->Frente->prox;
        Fila->Final->prox = NULL;
    }
    else
    {
        Apontador p = Fila->Frente->prox;
        Apontador pAnt = p;
        while (p != NULL)
        {
            if(celula->peso < p->peso)
            {
                if(p != pAnt)
                {
                    pAnt->prox = celula;
                    celula->prox = p;
                }
                else
                {
                    celula->prox = p;
                    Fila->Frente->prox = celula;
                }
                break;
            }
            if(p!=pAnt)
            {
                pAnt=pAnt->prox;
            }
            p=p->prox;
        }
        if(p == NULL)
        {
            Fila->Final->prox = celula;
            Fila->Final = Fila->Final->prox;
            Fila->Final->prox = NULL;
        }
    }
    Fila->Tamanho = Fila->Tamanho + 1;
}

//RETIRA O PRIMEIRO ELEMENTO (MENOR FREQUENCIA) DA LISTA
Apontador ExtraiMin(struct TipoFila *Fila)
{
    Apontador primeiro = Fila->Frente->prox;
    Apontador proximo = primeiro->prox;
    primeiro->prox = NULL;
    Fila->Tamanho = Fila->Tamanho - 1;
    if(Fila->Tamanho != 0)
    {
        Fila->Frente->prox = proximo;
    }
    else
    {
        Fila->Final = Fila->Frente;
        Fila->Frente->prox = NULL;
        Fila->Tamanho = 0;
    }
    return(primeiro);
}

//GERA OS CÓDIGOS DAS FOLHAS DA ARVORE BINÁRIA GERADA PELO CODIGO DE HUFFMAN, OS QUAIS REPRESENTAM AS CORES EM FORMA BINÁRIA
void GeraCodigos(Apontador no, codigo *codigos, int compacta)
{
    if(no != NULL)
    {
        int tam = strlen(no->ficheiro);
        if(no->fesq != NULL)
        {
            no->fesq->ficheiro = (char*) malloc((300)*sizeof(char));
            no->fesq->ficheiro[0] = '\0';
            strcpy(no->fesq->ficheiro,no->ficheiro);
            strcat(no->fesq->ficheiro,"0");
            GeraCodigos(no->fesq, codigos, compacta);
        }
        if(no->fdir != NULL)
        {
            no->fdir->ficheiro = (char*) malloc((300)*sizeof(char));
            no->fdir->ficheiro[0] = '\0';
            strcpy(no->fdir->ficheiro,no->ficheiro);
            strcat(no->fdir->ficheiro,"1");
            GeraCodigos(no->fdir, codigos, compacta);
        }
        if(no->fesq == NULL && no->fdir == NULL)
        {
            int aux;
            aux = no->cor;
            codigos[aux].cod = (char*) malloc((300)*sizeof(char));
            if(maior<tam)
            {
                maior = tam;
            }
            codigos[aux].cod[0] = '\0';
            codigos[aux].peso = no->peso;
            strcpy(codigos[aux].cod,no->ficheiro);
        }
    }

}

//------------------------------------------------------
//   Converte uma string em binario para um valor char
//------------------------------------------------------

unsigned char bintochar( char* binstr )
{
    // Retorna o valor char que a string passada como argumento
    // representa. Utiliza a funcao strtoul da biblioteca padrao
   	unsigned char retorno = (unsigned char) ( strtol( binstr, NULL, 2 ) );
   	
    return retorno; 
}


//----------------------------------------------------------
//  Converte um valor do tipo char para uma string binaria
//----------------------------------------------------------

char* chartobin ( unsigned char c )
{
    static char bin[CHAR_BIT + 1] = { 0 };
    int i;

    for ( i = CHAR_BIT - 1; i >= 0; i-- )
    {
        bin[i] = (c % 2) + '0';
        c /= 2;
    }

    // Retorna a string com a representacao binaria do argumento

    return bin;
}


//DECLARA UMA ESTRUTURA DO TIPO DADO, O QUAL SERÁ AUXILIAR NA HORA DA EXECUCAO DAS THREADS
struct tipoDado{
	int L;
	int C;
	unsigned char* string;
	int tamanho;
	int** matriz;
	int numthreads;
	unsigned long long qtcar;
	int indice;
};

codigo codigos[256]; //DECLARA O HASH DE CODIGOS BINARIOS

tipoDado dados[NUMTHREADS]; //DECLARA UM VETOR DO TIPO DADO



/*****************************************************************|
|*******************.....THREADS' ALGORITHM.....******************|
|***********************....COMPRESSÃO...*************************|
/*****************************************************************|
|***********************.....RUN LENGTH.....**********************|
|***********************....COM CONTROLE...***********************|
|*****************************************************************/

void *compacta(void *id){
	long int id2 = (long int)id;
	tipoDado *dado = &dados[id2];
	long long i, j;
	int c=-1, ca=-1;
    char* temp = (char*)malloc(sizeof(char));
    temp[8] = '\0';
    unsigned char tst;
    long long cont=0;
    int t,tr, repet=0, w, y=0;
    long long siz = (long long)((dado->L)*(dado->C)/NUMTHREADS)*(maior+2);
    dado->string = (unsigned char*)malloc(siz*sizeof(char));
    dado->string[0]='\0';
	for(i=(dado->indice)*(dado->L)/NUMTHREADS; i<(dado->indice+1)*(dado->L)/NUMTHREADS; i++)
    {
        for(j=0; j<dado->C; j++)
        {
            c = dado->matriz[i][j];
            if(repet==255 || (repet>3 && c!=ca))
			{
				tr = strlen(codigos[controle].cod);
                for(w=0; w<tr; w++)
                {
                    dado->qtcar++;
                    if(y == 7)
                    {
                        temp[y] = codigos[controle].cod[w];
                    	tst = bintochar(temp);
                        dado->string[cont] = tst;
                        cont++;
                        y=0;
					}
                    else
                    {                       
                        temp[y] = codigos[controle].cod[w];
                        y++;
                    }
                }
				tr = strlen(codigos[repet].cod);
                for(w=0; w<tr; w++)
                {
                    dado->qtcar++;
                    if(y == 7)
                    {
                        temp[y] = codigos[repet].cod[w];
                    	tst = bintochar(temp);
						dado->string[cont] = tst;
                        cont++;
                        y=0;
                    }
                    else
                    {                       
                        temp[y] = codigos[repet].cod[w];
                        y++;
                    }
                }
                t = strlen(codigos[ca].cod);
                for(w=0; w<t; w++)
                {
                    dado->qtcar++;
                    if(y == 7)
                    {
                        temp[y] = codigos[ca].cod[w];
                    	tst = bintochar(temp);
						dado->string[cont] = tst;
                        cont++;
                        y=0;
                    }
                    else
                    {
                        temp[y] = codigos[ca].cod[w];
                        y++;
                    }
                }
                repet=0;
			}
			else if(0<repet && repet<4 && c!=ca)
			{
				if(ca == controle){
					tr = strlen(codigos[controle].cod);
	                for(w=0; w<tr; w++)
	                {
	                    dado->qtcar++;
	                    if(y == 7)
	                    {
	                        temp[y] = codigos[controle].cod[w];
	                    	tst = bintochar(temp);
							dado->string[cont] = tst;
	                        cont++;
	                        y=0;
	                    }
	                    else
	                    {                       
	                        temp[y] = codigos[controle].cod[w];
	                        y++;
	                    }
	                }
					tr = strlen(codigos[repet].cod);
	                for(w=0; w<tr; w++)
	                {
	                    dado->qtcar++;
	                    if(y == 7)
	                    {
	                        temp[y] = codigos[repet].cod[w];
	                    	tst = bintochar(temp);
							dado->string[cont] = tst;
	                        cont++;
	                        y=0;
	                    }
	                    else
	                    {                       
	                        temp[y] = codigos[repet].cod[w];
	                        y++;
	                    }
	                }
	                t = strlen(codigos[ca].cod);
	                for(w=0; w<t; w++)
	                {
	                    dado->qtcar++;
	                    if(y == 7)
	                    {
	                        temp[y] = codigos[ca].cod[w];
	                    	tst = bintochar(temp);
							dado->string[cont] = tst;
	                        cont++;
	                        y=0;
	                    }
	                    else
	                    {
	                        temp[y] = codigos[ca].cod[w];
	                        y++;
	                    }
	                }
	                repet=0;
				}
				else
				{
					t = strlen(codigos[ca].cod);
					for(int aux=repet; aux>0;aux--){
			            for(w=0; w<t; w++)
			            {
		                    dado->qtcar++;
			                if(y == 7)
			                {
			                    temp[y] = codigos[ca].cod[w];
			                	tst = bintochar(temp);
								dado->string[cont] = tst;
			                    cont++;
			                    y=0;
			                }
			                else
			                {
			                    temp[y] = codigos[ca].cod[w];
			                    y++;
			                }
			            }
			        }
	                repet=0;
				}
			}
			ca = c;
       		repet++;
   			if((dado->L-1) == i && (dado->C-1)==j){
   				if(repet==255 || (repet>3))
				{
					tr = strlen(codigos[controle].cod);
	                for(w=0; w<tr; w++)
	                {
	                    dado->qtcar++;
	                    if(y == 7)
	                    {
	                        temp[y] = codigos[controle].cod[w];
	                    	tst = bintochar(temp);
							dado->string[cont] = tst;
	                        cont++;
	                        y=0;
	                    }
	                    else
	                    {                       
	                        temp[y] = codigos[controle].cod[w];
	                        y++;
	                    }
	                }
					tr = strlen(codigos[repet].cod);
	                for(w=0; w<tr; w++)
	                {
	                    dado->qtcar++;
	                    if(y == 7)
	                    {
	                        temp[y] = codigos[repet].cod[w];
	                    	tst = bintochar(temp);
							dado->string[cont] = tst;
	                        cont++;
	                        y=0;
	                    }
	                    else
	                    {                       
	                        temp[y] = codigos[repet].cod[w];
	                        y++;
	                    }
	                }
	                t = strlen(codigos[ca].cod);
	                for(w=0; w<t; w++)
	                {
	                    dado->qtcar++;
	                    if(y == 7)
	                    {
	                        temp[y] = codigos[ca].cod[w];
	                    	tst = bintochar(temp);
							dado->string[cont] = tst;
	                        cont++;
	                        y=0;
	                    }
	                    else
	                    {
	                        temp[y] = codigos[ca].cod[w];
	                        y++;
	                    }
	                }
				}
				else if(0<repet && repet<4)
				{
					if(ca == controle){
						tr = strlen(codigos[controle].cod);
			            for(w=0; w<tr; w++)
			            {
		                    dado->qtcar++;
			                if(y == 7)
			                {
			                    temp[y] = codigos[controle].cod[w];
			                	tst = bintochar(temp);
								dado->string[cont] = tst;
			                    cont++;
			                    y=0;
			                }
			                else
			                {                       
			                    temp[y] = codigos[controle].cod[w];
			                    y++;
			                }
			            }
						tr = strlen(codigos[repet].cod);
			            for(w=0; w<tr; w++)
			            {
		                    dado->qtcar++;
			                if(y == 7)
			                {
			                    temp[y] = codigos[repet].cod[w];
			                	tst = bintochar(temp);
								dado->string[cont] = tst;
			                    cont++;
			                    y=0;
			                }
			                else
			                {                       
			                    temp[y] = codigos[repet].cod[w];
			                    y++;
			                }
			            }
			            t = strlen(codigos[ca].cod);
			            for(w=0; w<t; w++)
			            {
		                    dado->qtcar++;
			                if(y == 7)
			                {
			                    temp[y] = codigos[ca].cod[w];
			                	tst = bintochar(temp);
								dado->string[cont] = tst;
			                    cont++;
			                    y=0;
			                }
			                else
			                {
			                    temp[y] = codigos[ca].cod[w];
			                    y++;
			                }
			            }
					}
					else
					{
						t = strlen(codigos[ca].cod);
						for(int aux=repet; aux>0;aux--){
					        for(w=0; w<t; w++)
					        {
			                    dado->qtcar++;
					            if(y == 7)
					            {
					                temp[y] = codigos[ca].cod[w];
					            	tst = bintochar(temp);
									dado->string[cont] = tst;
					                cont++;
					                y=0;
					            }
					            else
					            {
					                temp[y] = codigos[ca].cod[w];
					                y++;
					            }
					        }
					    }
					}
				}
   			}
        }
    }
    if(y!=0)
	{
		for(i=0; i<y; i++)
		{
		    dado->string[cont] = (unsigned char)temp[i];
		    cont++;
		}
	}
    dado->string[cont] = '\0';
    dado->tamanho = cont;
    pthread_exit(NULL);
};


TipoArvore arvore;

//---------------------------------------------------------------MAIN || PRINCIPAL------------------------------------------------------------//

/*****************************************************************|
|**************************.....MAIN.....*************************|
|*************************....PRINCIPAL...************************|
|*****************************************************************/

int** cimg; //declara matriz de cores

int main ()
{
	pthread_t threads[NUMTHREADS];
    char comando[4];
    int L,C,maxi,cor=-1, coranterior=-1,repeticoes=0;
	long long i,j;
    scanf("%3s",comando);
    scanf("%d %d", &L, &C);
    getchar();
    cimg = new int*[L];
    for (i = 0; i < L; ++i)
    {
        cimg[i] = new int[C];
    }

    scanf("%d",&maxi);
    getchar();
    long long contarep[maxi+1];
    for (i=0; i<maxi+1; i++)
    {
        contarep[i]=0;
    }
//-----------------------------------------------IMAGEM NÃO ESTÁ COMPACTADA, É A ORIGINAL!--------------------------------------------------------------------//
   
/*****************************************************************|
|***********************.....RUN LENGTH.....**********************|
|***********************....COM CONTROLE...***********************|
|*****************************************************************/

    if(strcmp(comando,"P2")==0)
    {
        printf("cP2\n%d %d\n%d\n",L,C,maxi);
        for(i=0; i<L; i++)
        {
            for(j=0; j<C; j++)
            {
                scanf("%d",&cor);
                cimg[i][j] = cor;
                if(repeticoes==255 || (repeticoes>3 && cor!=coranterior))
				{
					contarep[controle]++;
					contarep[repeticoes]++;
					contarep[coranterior]++;
					repeticoes = 0;
				}
				else if(0<repeticoes && repeticoes<4 && cor!=coranterior)
				{
					if(coranterior == controle){
						contarep[controle]++;
						contarep[repeticoes]++;
						contarep[coranterior]++;
					}
					else
					{
						contarep[coranterior] += repeticoes;
					}
					repeticoes = 0;
				}
				coranterior = cor;
	       		repeticoes++;
       			if((L-1) == i && (C-1)==j){
       				if(repeticoes==255 || (repeticoes>3))
					{
						contarep[controle]++;
						contarep[repeticoes]++;
						contarep[coranterior]++;
					}
					else if(0<repeticoes && repeticoes<4)
					{
						if(coranterior == controle){
							contarep[controle]++;
							contarep[repeticoes]++;
							contarep[coranterior]++;
						}
						else
						{
							contarep[coranterior] += repeticoes;
						}
					}
       			}
       		} 
       	}


/*****************************************************************|
|***********************.....HUFFMAN.....**********************|
|*****************************************************************/


        int nroFolhas=0;
        TipoFila Q;
        FazFilaVazia(&Q);
        for(i=0; i<maxi+1; i++)
        {
            if(contarep[i]>0)
            {
                nroFolhas++;
                Apontador cell = (Apontador)malloc(sizeof(Celula));
                cell->peso = contarep[i];
                cell->cor = i;
                cell->fdir = NULL;
                cell->fesq = NULL;
                cell->prox = NULL;
                Insere(cell, &Q); //INSERE NA FILA DE FORMA ORDENADA
            }
        }
        Apontador S0, S1;
        while(Q.Tamanho>1)
        {
            S0 = ExtraiMin(&Q); //EXTRAI O PRIMEIRO DA FILA, POIS ELA JÁ ESTÁ ORDENADA DE FORMA CRESCENTE COM RELAÇÃO AO PESO/FREQUENCIA
            S1 = ExtraiMin(&Q);
            Apontador X = (Apontador)malloc(sizeof(Celula));
            X->fesq = S0;
            X->fdir = S1;
            X->cor = -1;
            X->peso = S0->peso + S1->peso;
            X->prox = NULL;
            if(Q.Tamanho==0)
            {
                X->ficheiro = (char*) malloc(300*sizeof(char));
                X->ficheiro[0] = '\0';
            }
            Insere(X,&Q);
        }
        arvore.Raiz = ExtraiMin(&Q);
        printf("%d\n",nroFolhas);
        GeraCodigos(arvore.Raiz, codigos, 1); //GERA OS CÓDIGOS DAS FOLHAS DA ARVORE BINÁRIA GERADA PELO CODIGO DE HUFFMAN,
        for(i=0; i<maxi+1; i++)				  //OS QUAIS REPRESENTAM AS CORES EM FORMA BINÁRIA
        {
            if(contarep[i]>0)
            {
            	printf("%d %llu\n",(int)i, contarep[i]);
            }
        }
	
/*****************************************************************|
|***********************.....COMPRESSÃO.....**********************|
|*****************************************************************/

		for(int bx = 0; bx < NUMTHREADS; bx++){
			dados[bx].L = L;
			dados[bx].C = C;
			dados[bx].matriz = cimg;
			dados[bx].numthreads = NUMTHREADS;
			dados[bx].qtcar = 0;
			dados[bx].tamanho = 0;
			dados[bx].indice = bx;
		}
		
		//CRIA AS THREADS
		for(int bx = 0; bx < NUMTHREADS; bx++){
			pthread_create(&threads[bx], NULL, compacta, (void *) bx);
		}
		//AGUARDA PELAS THREADS TERMINAREM
		for(int bx = 0;bx < NUMTHREADS;bx++){
			pthread_join(threads[bx], NULL);
		}
		printf("%d\n",NUMTHREADS); //IMPRIME O NUMERO DE THREADS PARA, NA HORA DA DESCOMPRESSAO, SABER-SE QUAL O NUMERO DE THREADS UTILIZADAS
		
		for(int bx=0;bx<NUMTHREADS;bx++){
			printf("%llu",dados[bx].qtcar); //IMPRIME A QUANTIDADE DE BITS QUE CADA THREAD PROCESSOU
			if(bx+1 != NUMTHREADS){
				printf(" ");
			}
		}
		printf("\n");
		for(int bx=0;bx<NUMTHREADS;bx++){
			for(i=0;i<dados[bx].tamanho;i++){
				printf("%c",dados[bx].string[i]); //IMPRIME A SEQUENCIA DE CHARS QUE REPRESENTAM O BITSTREAM DA IMAGEM CODIFICADA
			}
		}
    }
    
//-----------------------------------------------IMAGEM JÁ ESTÁ COMPACTADA, DESCOMPACTAR!-----------------------------------------------------------------------//

/*****************************************************************|
|********************.....DESCOMPRESSÃO.....**********************|
|*****************************************************************/
    else if(strcmp(comando,"cP2")==0)
    {
		printf("P2\n%d %d\n%d\n",L,C,maxi);
		int qtde;
		long long repeat;
		int THREADS;
		int color;
		scanf("%d",&qtde);
		getchar();
		
/*****************************************************************|
|************************.....HUFFMAN.....************************|
|*****************************************************************/
		TipoFila Q;
        FazFilaVazia(&Q);
        for(i=0; i<qtde; i++)
        {
        	scanf("%d %llu",&color,&repeat);
        	getchar();
            Apontador cell = (Apontador)malloc(sizeof(Celula));
            cell->peso = repeat;
            cell->cor = color;
            cell->fdir = NULL;
            cell->fesq = NULL;
            cell->prox = NULL;
            Insere(cell, &Q);
        }
        Apontador S0, S1;
        while(Q.Tamanho>1)
        {
            S0 = ExtraiMin(&Q);
            S1 = ExtraiMin(&Q);
            Apontador X = (Apontador)malloc(sizeof(Celula));
            X->fesq = S0;contarep[i];
            X->fdir = S1;
            X->cor = -1;
            X->peso = S0->peso + S1->peso;
            X->prox = NULL;
            if(Q.Tamanho==0)
            {
                X->ficheiro = (char*) malloc(sizeof(char));
                X->ficheiro[0] = '\0';
            }
            Insere(X,&Q);
        }
        arvore.Raiz = ExtraiMin(&Q);
        codigo codigos[maxi+1];
        GeraCodigos(arvore.Raiz, codigos, 0);
		unsigned char byte;
		long long cont=0;
		scanf("%d",&THREADS);
		getchar();
		unsigned long long qtbit[THREADS];
		for(i=0;i<THREADS;i++){
			scanf("%llu",&qtbit[i]);
			getchar();
		}
		Apontador no = arvore.Raiz;
		int vezes, coraux, flag = 0;
		int contador=1;
		char tmp[9];
		tmp[0] = '\0';
		char bit;
/*****************************************************************|
|********************.....DECODIFICAÇÃO.....**********************|
|*****************************************************************/
		for(j=0;j<THREADS;j++){
			i=0;
			while(i<qtbit[j]){
			 	cont=i;
		    	if((qtbit[j] - (cont+1)) < (qtbit[j]%8)){
		    		scanf("%c",&byte);
		    		if(byte == '1'){
			    		no = no->fdir;
			    	}else{
			    		no = no->fesq;
			    	}
			    	if(no->fesq == NULL && no->fdir == NULL){
			    		if(no->cor==controle && flag == 0)
						{
							flag = 1;
						}else if(flag == 1){
							flag = 2;
							vezes = no->cor;
						}else if(flag == 2){
							flag = 0;
							coraux = no->cor;
							for(int ax=0;ax<vezes;ax++){
								printf("%d",coraux);
								if(contador == L){
									contador=0;
									printf("\n");
								}else{
									printf(" ");
								}
								contador++;
							}
						}else{
							flag = 0;
							coraux = no->cor;
							printf("%d",coraux);
							if(contador == L){
								contador=0;
								printf("\n");
							}else{
								printf(" ");
							}
							contador++;
						}
			    		no = arvore.Raiz;
			    	}
		    	}else{
		    		if((cont+1)%8 == 0){
		    			scanf("%c",&byte);
			   		 	strcpy(tmp,chartobin(byte));
			   		 	for(int ax = 0; ax<8; ax++){
			   		 		bit = tmp[ax];
			   		 		if(bit == '1'){
								no = no->fdir;
							}else{
								no = no->fesq;
							}
							if(no->fesq == NULL && no->fdir == NULL){
								if(no->cor==controle && flag==0)
								{
									flag = 1;
								}else if(flag == 1){
									flag = 2;
									vezes = no->cor;
								}else if(flag == 2){
									flag = 0;
									coraux = no->cor;
									for(int ax=0;ax<vezes;ax++){
										printf("%d",coraux);
										if(contador == L){
											contador=0;
											printf("\n");
										}else{
											printf(" ");
										}
										contador++;
									}
								}else{
									flag = 0;
									coraux = no->cor;
									printf("%d",coraux);
									if(contador == L){
										contador=0;
										printf("\n");
									}else{
										printf(" ");
									}
									contador++;
								}
								no = arvore.Raiz;
							}
			   		 	}
			   		}
		   		}
		    	i++;
		    }
		}
    }
    return 0;
}


