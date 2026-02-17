#include <stdio.h>

typedef struct fa
{
    int adat;
    struct fa *bal, * jobb
}node;

void beszur(int d, node *gy){ //ez az egész rossz!!!
    if (gy==NULL){
        gy=(node*)malloc(sizeof(node));
        (*gy).adat=d; //gy->adat=d;
        (*gy).bal=NULL;
        (*gy).jobb=NULL;
    }
    if((*gy).adat==d) return;
    if((*gy).adat>d) beszur(d,(*gy).bal);
    else beszur(d,(*gy).jobb);
}

int main(){
    node *gy=NULL;
    beszur(123,gy);

    return 0;
}