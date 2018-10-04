#include <iostream>
#include <stdio.h>
#include <ctime>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <vector>
#include <math.h>
#include <sys/time.h>

float* terreno; //Array do terreno
int resol; //resolucao do terreno(tamanho)
float interv; // intervalo de alturas
float fatRugosidade;// fator de rugosidade
float escala; //escala ( para movimentacao da camera)
float anguloX, anguloY; //para rotacao da camera
float mapaH;
float hmPixPos;
float maxAlt, minAlt, range; //ponto mais alto, mais baixo e sua diferença
float luz1Pos[3], luz2Pos[3]; //vetor de pos da luz
int luzPos; //luz selecionada
int win1, win2; //id de janela
bool wireframe = false; //liga ou desliga wireframe
bool ligaAgua = false; // adiciona ou nao agua
bool ligacor = false; // modo colorido ligado/desligado

std::vector<float> normals; //vetor de normais para calculo de shading


std::vector<float> calcTriNormal (float t1x, float t1y, float t1z, float t2x, float t2y, float t2z, float t3x, float t3y, float t3z) {
//calcula a normal de um triangulo dado 3 pontos
	std::vector<float> norm (3,0);

	//vector 1
	float v1x = t2x-t1x;
	float v1y = t2y-t1y;
	float v1z = t2z-t1z;
	//vector 2
	float v2x = t3x-t1x;
	float v2y = t3y-t1y;
	float v2z = t3z-t1z;

	norm[0]=v1y*v2z-v1z*v2y;
	norm[1]=v1z*v2x-v1x*v2z;
	norm[2]=v1x*v2y-v1y*v2x;

	return norm;
}
float vLength(std::vector<float> vec) { //calcula tam do vetor (normalizado)
	return sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
}

std::vector<float> calcNormals (float* terr, int w, int l) { //calcula as normais dos vertices
	std::vector<float> norms (w*l*3,0);

	for (int z=0; z<l; z++) {
		for (int x=0; x<w; x++) {
			std::vector<float> vecs (3,0);

			if (z>0 && x>0) {
				std::vector<float> n;
				n = calcTriNormal(x,terr[z*l+x],z,x,terr[(z-1)*l+x],z-1,x-1,terr[z*l+x-1],z);
				float l=vLength(n);
				vecs[0]+=n[0]/l;
				vecs[1]+=n[1]/l;
				vecs[2]+=n[2]/l;
			}
			if (x>0 && z<l-1) {
				std::vector<float> n;
				n = calcTriNormal(x,terr[z*l+x],z,x-1,terr[z*l+x-1],z,x,terr[(z+1)*l+x],z+1);
				float l=vLength(n);
				vecs[0]+=n[0]/l;
				vecs[1]+=n[1]/l;
				vecs[2]+=n[2]/l;
			}
			if (z<l-1 && x<w-1) {
				std::vector<float> n;
				n = calcTriNormal(x,terr[z*l+x],z,x,terr[(z+1)*l+x],z+1,x+1,terr[z*l+x+1],z);
				float l=vLength(n);
				vecs[0]+=n[0]/l;
				vecs[1]+=n[1]/l;
				vecs[2]+=n[2]/l;
			}
			if (x<w-1 && z>0) {
				std::vector<float> n;
				n = calcTriNormal(x,terr[z*l+x],z,x+1,terr[z*l+x+1],z,x,terr[(z-1)*l+x],z-1);
				float l=vLength(n);
				vecs[0]+=n[0]/l;
				vecs[1]+=n[1]/l;
				vecs[2]+=n[2]/l;
			}
			float vl=vLength(vecs);
			norms[(z*l+x)*3]=vecs[0]/vl;
			norms[(z*l+x)*3+1]=vecs[1]/vl;
			norms[(z*l+x)*3+2]=vecs[2]/vl;


		}
	}

	return norms;
}


float* iniciaTerreno (int width, int length) { //gera plano com y = 5
	float *terreno = new float[width*length];

	for (int i=0; i<width; i++) {
		for (int j=0; j<length; j++) {
			terreno[i*width+j] = 5;
		}
	}
	return terreno;
}

//APLICAÇÃO DE COR AO TERRENO
void colorirVert(float altVert) {
	float altInterv;
    altInterv = (altVert-minAlt)/range;
	if (ligacor) { // com cor
		if (altInterv<0.3) {                       //areia
			glColor3f(0.93,0.86,0.51);
		}
		else if (altInterv>0.3 && altInterv<0.5) {    //grama
			glColor3f(0.0,0.5,0.0);
		}
		else if (altInterv>0.5 && altInterv<0.7) {    //montanha
			glColor3f(0.55,0.45,0.33);
		}
		else {                                  //neve
			glColor3f(1,1,1);
		}
	} else { // se nao tiver cor, usa tons de cinza
        glColor3f(altInterv, altInterv, altInterv);
	}
}

void colorirAgua(float R, float G, float B){
    float cinza;
    if(ligacor){
        glColor3f(R, G, B);
    } else{
        cinza = (R + G + B) / (float) 3;
        glColor3f(cinza, cinza, cinza);
    }
}

//RENDERIZAÇÃO DO TERRENO
void renderTerreno(float* t, int width, int length) {

    float vertA, vertB, altAgua, R, G, B;
    int indVertA, indVertB;


    if(ligaAgua){

        altAgua = (0.21 * range) + minAlt;

        glBegin(GL_QUADS);
            R = 0.29;
            G = 0.50;
            B = 0.85;

            //glColor3f(0.29, 0.50, 0.85);


            colorirAgua(R, G, B);
            //glColor4f(0.29, 0.50, 0.85, 0.5);
            glNormal3f(0.0, 1.0, 0.0);
            glVertex3f(0, altAgua, length);

            colorirAgua(R, G, B);
            //glColor4f(0.29, 0.50, 0.85, 0.5);
            glNormal3f(0.0, 1.0, 0.0);
            glVertex3f(width, altAgua, length);

            colorirAgua(R, G, B);
            //glColor4f(0.29, 0.50, 0.85, 0.5);
            glNormal3f(0.0, 1.0, 0.0);
            glVertex3f(width, altAgua, 0);

            colorirAgua(R, G, B);
            //glColor4f(0.29, 0.50, 0.85, 0.5);
            glNormal3f(0.0, 1.0, 0.0);
            glVertex3f(0, altAgua, 0);
        glEnd();
    }


	for (int l=0; l<length-1; l++) {
        glBegin(GL_TRIANGLE_STRIP);

        for (int w = 0; w < width; w++) {

            indVertA = (l*length) + w;
            indVertB = ((l+1)*length) + w;

            vertA = t[indVertA];
            vertB = t[indVertB];

            //printf("%f\n", vertA);

            colorirVert(vertA);
            glNormal3f(normals[indVertA * 3], normals[indVertA * 3 + 1], normals[indVertA * 3 + 2]);
            //printf("%f :: %f :: %f\n", normals[indVertA * 3], normals[indVertA * 3 + 1], normals[indVertA * 3 + 2]);
            glVertex3f(w, vertA, l);

            colorirVert(vertB);
            glNormal3f(normals[indVertB * 3], normals[indVertB * 3 + 1], normals[indVertB * 3 + 2]);
            glVertex3f(w, vertB, l+1);
        }
        glEnd();
    }

}


float maiorAlt (float* t, int w, int l) {
	float maximo = 10;
	for (int i=0; i<w; i++) {
		for (int j=0; j<l; j++) {
			if (t[i*w+j] > maximo) maximo = t[i*w+j];
		}
	}
	return maximo;
}
float menorAlt (float* t, int w, int l) {
	float minimo = 10;
	for (int i=0; i<w; i++) {
		for (int j=0; j<l; j++) {
			if (t[i*w+j] < minimo) minimo = t[i*w+j];
		}
	}
	return minimo;
}

/*
float randRange(float piso, float teto) {
  return ((rand()%100)/100) * (teto - piso) + piso;
}
*/
double GetRandom() {
    double retorno = (rand()/(double)RAND_MAX) *2 - 1;//(double) ((rand()/RAND_MAX)*2) - 1.0;
    //printf("%f\n",retorno);
    return retorno; //(double) ((rand()/RAND_MAX)*2) - 1.0;

}

int getPosicaoMatriz(int lin, int col, int tam){
    int pos = tam * lin + col;
    //if (pos >= 0 && pos < (tam*tam)){
        return pos;
    //} else if(pos < 0){
    //    return 0;
    //} else{
    //    return tam*tam-1;
    //}
}


/*
void diamondSquare(float * mapAlt, int tam){

    //zValues => mapAlt

    // Initialize the four corners with a pre-seeded value
    //float zValues[tam][tam];

    mapAlt[getPosicaoMatriz(0,0,tam)] = 0.05;
    mapAlt[getPosicaoMatriz(tam-1,0,tam)] = 0.05;
    mapAlt[getPosicaoMatriz(0,tam-1,tam)] = 0.05;
    mapAlt[getPosicaoMatriz(tam-1,tam-1,tam)] = 0.05;

    // Diamond-Square algorithm
    int halfStep, stepSize = tam - 1;
    float lower = -0.1;
    float upper = 0.1;
    float topLeft, topRight, botLeft, botRight, left, right, up, down, avg, rand;

    int x, y, xStart;

    while (stepSize > 1) {
        halfStep = stepSize / 2;

        for (y = 0; y < tam; y += stepSize) {
            for (x = 0; x < tam; x += stepSize) {
                topLeft  = mapAlt[getPosicaoMatriz(x,y,tam)];
                topRight = mapAlt[getPosicaoMatriz(x + stepSize,y,tam)];
                botLeft  = mapAlt[getPosicaoMatriz(x,y + stepSize,tam)];
                botRight = mapAlt[getPosicaoMatriz(x + stepSize,y + stepSize,tam)];

                avg = (topLeft + topRight + botLeft + botRight) / 4;
                rand = randRange(lower, upper); //numero randomico com max e min
                mapAlt[getPosicaoMatriz(x + halfStep,y + halfStep,tam)] = avg + rand;
            }
        }

        int even = 1;
        for (y = 0; y < tam; y += halfStep) {
            xStart = even ? 0 : halfStep;
            for (x = xStart; x < tam; x += halfStep) {
                left  = ((x - halfStep) < 0)  ? 0 : mapAlt[getPosicaoMatriz(x - halfStep,y,tam)];
                right = ((x + halfStep) >= tam) ? 0 : mapAlt[getPosicaoMatriz(x + halfStep,y,tam)];
                up    = mapAlt[getPosicaoMatriz(x,y + halfStep,tam)];
                //printf("x - %d , y - %d , halfStep - %d\n",x , y, halfStep);
                down  = mapAlt[getPosicaoMatriz(x,y - halfStep,tam)];
                //printf("Down- %d\n", down);

                avg = (left + right + up + down) / 4;
                rand = randRange(lower, upper);
                mapAlt[getPosicaoMatriz(x,y,tam)] = avg + rand;
            }
            if(even == 1)
                even = 0;
            else
                even = 1;
        }

        stepSize /= 2;
        lower += 0.005;
        upper -= 0.005;
    }

}

*/


// ALGORITMO DO DIAMONDSQUARE
void diamondSquare(float * mapAlt, int tam, unsigned x1, unsigned y1, unsigned x2, unsigned y2, float interv, unsigned level, float r) {
    if (level < 1) return;

    unsigned i, j;

    // diamonds
    for (i = x1 + level; i < x2; i += level)
        for (j = y1 + level; j < y2; j += level) {
            float a = mapAlt[getPosicaoMatriz( i - level, j - level, tam)];
            float b = mapAlt[getPosicaoMatriz(i, j - level, tam)];
            float c = mapAlt[getPosicaoMatriz(i - level, j, tam)];
            float d = mapAlt[getPosicaoMatriz(i, j, tam)];
            float e = mapAlt[getPosicaoMatriz(i - level / 2, j - level / 2, tam)] = (a + b + c + d) / 4 + GetRandom() * interv;
        }

    // squares
    for (i = x1 + 2 * level; i < x2; i += level)
        for (j = y1 + 2 * level; j < y2; j += level) {
            float a = mapAlt[getPosicaoMatriz(i - level, j - level, tam)];
            float b = mapAlt[getPosicaoMatriz(i, j - level, tam)];
            float c = mapAlt[getPosicaoMatriz(i - level, j, tam)];
            float d = mapAlt[getPosicaoMatriz(i, j, tam)];
            float e = mapAlt[getPosicaoMatriz(i - level / 2, j - level / 2, tam)];

            float f = mapAlt[getPosicaoMatriz(i - level, j - level / 2, tam)] = (a + c + e + mapAlt[getPosicaoMatriz(i - 3 * level / 2, j - level / 2, tam)]) / 4 + GetRandom() * interv;
            float g = mapAlt[getPosicaoMatriz(i - level / 2, j - level, tam)] = (a + b + e + mapAlt[getPosicaoMatriz(i - level / 2, j - 3 * level / 2, tam)]) / 4 + GetRandom() * interv;
        }

    interv *= pow(2, -r);
    diamondSquare(mapAlt, tam, x1, y1, x2, y2, interv / 2, level / 2, r);
}


void display() {

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);
    if (luzPos == 0){
            glLightfv(GL_LIGHT0, GL_POSITION, luz1Pos);
    } else if (luzPos == 1){
            glLightfv(GL_LIGHT0, GL_POSITION, luz2Pos);
    }


	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,20,40,0,0,0,0,1,-1); //muda angulo para ajustar camera

	glRotatef(anguloX,1,0,0); //rotaciona camera
	glRotatef(anguloY,0,1,0);



	glScalef(escala,escala,escala);
	glTranslatef(-(resol/2),0,-(resol/2));

	if(!wireframe){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else { //modo de wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	renderTerreno(terreno,resol,resol);

	glutSwapBuffers();
}
void display2() { //window 2 - heightmap

	glMatrixMode(GL_MODELVIEW);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glBegin(GL_POINTS); //itera cada valor de altura e define os pix
		for (int i=0; i<resol; i++) {
			for (int j=0; j<resol; j++) {
				mapaH = terreno[i*resol+j];

				glColor3f((mapaH - minAlt)/range,(mapaH - minAlt)/range,(mapaH - minAlt)/range); //colore de cinza baseando-se na altura
				glVertex2f(hmPixPos*i-1,hmPixPos*j-1);
			}
		}
	glEnd();
	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,w/h,1,200);
	glMatrixMode(GL_MODELVIEW);
}
void reshape2(int w, int h) {
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void refatoraMap() { // refatora o terreno
	delete[] terreno; //deleta o terreno
	terreno = iniciaTerreno(resol,resol); //gera-o denovo
	diamondSquare(terreno, resol, 0, 0, resol, resol, interv, resol, fatRugosidade);
	escala = 35.0f / resol;
	maxAlt = maiorAlt(terreno, resol, resol);
	minAlt = menorAlt(terreno, resol, resol);
	range = maxAlt - minAlt;
	normals = calcNormals(terreno, resol, resol);
}

void comandos(unsigned char key, int x, int y) {
    key = toupper(key);
	switch (key) {
		case 27:        // ESC
			exit(0);
			break;
		case 'L':       //MUDA POSICAO DA LUZ
			if(luzPos == 1){
                luzPos = 0;
			} else {
                luzPos = 1;
			}
			break;
		case 'R':       // REFATORA TERRENO
			refatoraMap();
			break;
		case 'W':       // LIGA WIREFRAME
			wireframe = !wireframe;
			break;
		case 'A':       // ADICIONA/REMOVE AGUA
			ligaAgua = !ligaAgua;
			break;
		case 'C':       // MUDA MODO DE COLORIZAÇÃO
			ligacor = !ligacor;
			break;
	}
	// recarrega os displays
	glutPostWindowRedisplay(win1);
	glutPostWindowRedisplay(win2);

}

void comandosGlut(int key, int x, int y) { //arrow keys
	switch (key) {
	    //Rotaciona a camera
		case GLUT_KEY_UP:
			anguloX += 5;
			break;
		case GLUT_KEY_DOWN:
			anguloX -= 5;
			break;
		case GLUT_KEY_LEFT:
			anguloY += 5;
			break;
		case GLUT_KEY_RIGHT:
			anguloY -= 5;
			break;
        // zoom in e out
		case GLUT_KEY_PAGE_UP:
			escala *= 1.1;
			break;
		case GLUT_KEY_PAGE_DOWN:
			escala *= 0.9;
			break;
	}
	glutPostWindowRedisplay(win1);
	glutPostWindowRedisplay(win2);
}


void init(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(1000,800);
	glutInitWindowPosition(10,20);
	win1 = glutCreateWindow("Terreno");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(comandos);
	glutSpecialFunc(comandosGlut);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);


	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	win2 = glutCreateWindow("Heightmap"); // janela de heightmap
	glutPositionWindow(1020,20);
	glutReshapeWindow(resol,resol);
	glutReshapeFunc(reshape2);
	glutDisplayFunc(display2);
	glutKeyboardFunc(comandos);
	glutSpecialFunc(comandosGlut);

}

int main(int argc, char** argv) {
    srand( (unsigned)time(NULL) );

	/*
	resol = 128; // resolução - deve que ser potência de 2
	interv = 300; // intervalo de alturas
	fatRugosidade = 1.0; // fator de rugosidade - menor = mais rugoso
    */

	resol = 512; // resolução - deve que ser potência de 2
	interv = 2000; // intervalo de alturas
	fatRugosidade = 1.0; // fator de rugosidade - menor = mais rugoso

	/*
	resol = 512; // resolução - deve que ser potência de 2
	interv = 100; // intervalo de alturas
	fatRugosidade = 0.1; // fator de rugosidade - menor = mais rugoso
	*/

	init(argc, argv);

	hmPixPos = 2.0 / resol;

	terreno = iniciaTerreno(resol,resol);

	diamondSquare(terreno, resol, 0, 0, resol, resol, interv, resol, fatRugosidade);

	normals = calcNormals(terreno, resol, resol);

	maxAlt = maiorAlt(terreno, resol, resol);
	minAlt = menorAlt(terreno, resol, resol);
	range = maxAlt - minAlt;
	escala = 35.0f/resol;

    //define as posicoes das luzes
	luz1Pos[0] = -(resol / 2);
	luz1Pos[1] = maxAlt / 2;
	luz1Pos[2] = -(resol / 2);

	luz2Pos[0] = (resol / 4);
	luz2Pos[1] = maxAlt / 2;
	luz2Pos[2] = -(resol / 4);


	glutMainLoop();
}
