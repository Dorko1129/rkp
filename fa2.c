#include <stdio.h>
#include <stdlib.h>
//include valami

typedef struct fa
{
    int adat;
    struct fa *bal, * jobb
}node;

void beszur(int d, node **gy){
    if (*gy==NULL){
        *gy=(node*)malloc(sizeof(node));
        (**gy).adat=d; //gy->adat=d;
        (**gy).bal=NULL;
        (**gy).jobb=NULL;
    }
    if((**gy).adat==d) return;
    if((**gy).adat>d) beszur(d,&((**gy).bal));
    else beszur(d,&((**gy).jobb));
}

void inorder(node *gy){
    if (gy==NULL) return;
    inorder((*gy).bal);
    printf("%d ",((*gy).adat));
    inorder((*gy).jobb);
}

int main(){
//ciklus random szamok
    node *gy=NULL;
    beszur(123,&gy);

    return 0;
}