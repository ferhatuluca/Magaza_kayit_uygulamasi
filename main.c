#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

/*  --AÇIKLAMA--
  
 * İsim için hash fonksiyonu ve seperate chaning çakışma algoritması kullandık.

 * Hash fonksiyonunu görebilmek için listeleme fonksiyonu ekledik.
  
 * ftruncate fonksiyonu verdiğimiz boyut kadar dosyaya null karakteri yazıyor.
   Bu sayede dosya belirli bir boyutta açılmış oluyor.
  
 * Tablo boyutunu global olarak tanımladık. Çakışmları artırmak için boyut düşürülebilir.
  (eski dosyalar silinmeli)
  
 * Magaza_id'ye sıfır değeri girilmemeli.
  
*/

int table_size = 100;

void menu();

typedef struct T_Magaza{
    char magaza_isim[20];
    char sehir[20];
    int gelir;
    int gider;
    int magaza_id;
    int sonraki;        //Sonraki elemanın adresi.(filepointer cinsinden)
}magaza;

typedef struct erisim_liste{
    int adres;
    struct erisim_liste *ileri ;
}e_liste;

e_liste *liste_basi = NULL;

e_liste * e_eleman_olustur(int adres){ 
    e_liste *a = (e_liste *)malloc(sizeof(e_liste));
    a->adres = adres;
    a->ileri = NULL;
}
  
void * e_eleman_ekle(int adres){    //Erişim listesine kayit ekleyen fonksiyon.
    e_liste *a = e_eleman_olustur(adres);
    if(liste_basi == NULL){
        a->ileri = liste_basi;
        liste_basi = a;
    }
    else{
        e_liste *x = liste_basi;
        while(x->ileri != NULL){
            x = x->ileri;
        }
        x->ileri = a;
    }
}

int e_eleman_sil(){            //Erişim listesinden bir eleman siler ve o elmanın adresini döner.
    int x = liste_basi -> adres;
    free(liste_basi);
    liste_basi = liste_basi->ileri;
    return x;
}

int hash_f(char *str){        //DJB2 hash fonksiyonu.
    int hash = 5381;
    int c;
    
    while (c = *str++){
        hash = ((hash << 5) + hash) + c; //hash*33 + c
    }
    
    return hash % table_size;
}

void kayit_ekleme()
{
    printf("********** Kayit Ekleme **********\n"); 
    FILE *fp;
    magaza k1;   // Eklenicek kayit
    magaza d1;   // Kayit tutucu
    int hash;
    
    
    if((fp = fopen("hash.bin", "rb+"))==NULL)
        printf("Dosya acilamadi!!!");
    
    printf("Magaza isim giriniz:  ");
    scanf("%s",k1.magaza_isim);
    
    printf("Sehir isimini giriniz:  ");
    scanf("%s",k1.sehir);
    
    printf("Geliri giriniz:  ");
    scanf("%d",&k1.gelir);
    
    printf("Gideri giriniz:  ");
    scanf("%d",&k1.gider);
    
    printf("Magaza id giriniz:  ");
    scanf("%d",&k1.magaza_id);    
    if(k1.magaza_id == 0){printf("magazaid 0 girildi"); return;}
    
    k1.sonraki = -1;	//NULL
    
    
    hash = hash_f(k1.magaza_isim);
    fseek(fp, hash*sizeof(magaza), SEEK_SET);
    fread(&d1, sizeof(magaza), 1, fp);
    fseek(fp, hash*sizeof(magaza), SEEK_SET);            //fread komutu fp'nin yerini değiştiriyor. Tekrar eski yerine alıyoruz.
    
    if(d1.magaza_id == NULL){      //Kayit alanı boşsa..                                                                                                                                            //eger kayit alanı bos ise. 
       fwrite(&k1, sizeof(magaza), 1, fp); 
    }
    else{                                               //Kayit alanı boş değilse bağlı listeye ekleme yapilir. 
        FILE *fp_chains = fopen("chains.bin","rb+");  

        if(d1.sonraki == -1){                           //O kayit adresinde daha önce çakışma olmamışsa..
            
            if(liste_basi != NULL){                     //Erişim listesinde eleman varsa..
                int x = e_eleman_sil();
                d1.sonraki = x;               
                fseek(fp_chains, x, SEEK_SET);
            }
            else{                                       //Erişim listesinde eleman yoksa ..  
                fseek(fp_chains, 0, SEEK_END); 
                d1.sonraki = ftell(fp_chains);
            }
                
            fwrite(&d1, sizeof(magaza), 1, fp);
            fwrite(&k1, sizeof(magaza), 1, fp_chains);
        }
        else{                                           //O kayit adresinde daha önce çakışma olmuşsa..
            int c1;    //yer tutucu
              
            do{                                         //Zincirin son elemanına gider.  
                fseek(fp_chains, d1.sonraki, SEEK_SET);
                c1 = ftell(fp_chains);
                fread(&d1, sizeof(magaza), 1, fp_chains);            
            }while(d1.sonraki != -1);
            
            if(liste_basi != NULL){                     //Erişim listesinde eleman varsa..
                int x = e_eleman_sil();
                d1.sonraki = x; 
                fseek(fp_chains, x, SEEK_SET);
            }
            else{                                      //Erişim listesinde eleman yoksa..
                fseek(fp_chains, 0, SEEK_END);
                d1.sonraki = ftell(fp_chains);
            }
            
            fwrite(&k1, sizeof(magaza), 1, fp_chains);  
            fseek(fp_chains, c1, SEEK_SET);
            fwrite(&d1, sizeof(magaza), 1, fp_chains);           
        }
        fclose(fp_chains);
    }
    fclose(fp);      
}

void sil()
{
    printf("********** Kayit Silme **********\n");
    FILE *fp = fopen("hash.bin", "rb+");
    magaza k1;              //Okunucak kayit.
    char isim[20];          //Silinicek isim.
    int hash;
    
    printf("Mağaza ismi:  ");
    scanf("%s", isim);
    
    hash = hash_f(isim);
    fseek(fp, hash*sizeof(magaza), SEEK_SET);
    fread(&k1, sizeof(magaza), 1, fp);
    fseek(fp, hash*sizeof(magaza), SEEK_SET);
    
    if(k1.magaza_id == NULL){                                    //Adres alanında kayıt yoksa..
        printf("Silinicek Kayit Bulunamadi!!!\n");
    }
    else if(!strcmp(k1.magaza_isim, isim) && k1.sonraki == -1 ){ //İsimler ayni ve kayidin gösterdigi başka bir kayit yoksa..
 
        fseek(fp, hash*sizeof(magaza), SEEK_SET);
        for (int i = 0; i<sizeof(magaza);i++){
            fputc('\0', fp);
        }
    }
    else if(!strcmp(k1.magaza_isim, isim) && k1.sonraki != -1 ){ //İsimler ayni ve kayidin gösterdigi başka bir kayit varsa..
        
        FILE *fp_chains = fopen("chains.bin", "rb+");
        magaza m1;                                                                                   
        
        fseek(fp_chains, k1.sonraki, SEEK_SET);
        fread(&m1, sizeof(magaza), 1, fp_chains);               
                                                                //Silinicek kayit ile home adresdeki
        e_eleman_ekle(k1.sonraki);                              //kayıdın yeri değiştirilir.
                                                                //Silinicek kayit erişim listesine eklenir.                               
        fwrite(&m1, sizeof(magaza), 1, fp); 
        fclose(fp_chains);
             
    }
    
    else{                                                       //İsimler ayni değilse çakışmış kayıtlara bakılır. 
        FILE *fp_chains = fopen("chains.bin", "rb+");
        fseek(fp_chains, k1.sonraki, SEEK_SET);
        fread(&k1, sizeof(magaza), 1, fp_chains);
        
        if(!strcmp(k1.magaza_isim, isim)){                      //Silinicek kayit home adresde ki kayittan hemen sonra gelen kayitsa..
            e_eleman_ekle(ftell(fp_chains) - sizeof(magaza));   
            int a = k1.sonraki;  //Yer tutucu
                               
            fread(&k1, sizeof(magaza), 1, fp);
            k1.sonraki = a;
            fseek(fp, hash*sizeof(magaza), SEEK_SET);
            fwrite(&k1, sizeof(magaza), 1, fp);
        }
        
        else{                                                  //Zincirin geri kalan kısmındaysa..
            int c1 = ftell(fp_chains)-sizeof(magaza);   //Yer tutucu
            int b = 0;          //kontrol değişkeni
            
            while(k1.sonraki != -1){                                //Zincirin elemanlarına dolaşır.
                fseek(fp_chains, k1.sonraki, SEEK_SET);
                fread(&k1, sizeof(magaza), 1, fp_chains);
                if(!strcmp(k1.magaza_isim, isim)){                  //Silinicek kaydı bulursa eğer döngüyü kırar.
                    b = 1;
                    break;
                }
                c1 = ftell(fp_chains) - sizeof(magaza);
            }
         
        if(b==1){                                                   //Silinicek kayıt bulunmuşsa..
            e_eleman_ekle(ftell(fp_chains)-sizeof(magaza));    
            int a = k1.sonraki;       //yer tutucu
            fseek(fp_chains, c1, SEEK_SET);
            fread(&k1, sizeof(magaza), 1, fp_chains);
            k1.sonraki = a;
            fseek(fp_chains, c1, SEEK_SET);
            fwrite(&k1, sizeof(magaza), 1, fp_chains);
        }
        else                                        
            printf("Silinicek Kayit Bulunamadi!!!\n");
        }
        
        fclose(fp_chains);
    }
    
    fclose(fp);
}

void silinmisleri_at(){
    FILE *fp = fopen("hash.bin", "rb+");
    FILE *fp_chains = fopen("chains.bin", "rb");
    FILE *fp_gecici = fopen("gecici.bin","wb");         //Geçici chains dosyası.
    magaza k1;
  
    for(int i = 0; i<table_size; i++){                  //Hash tablosunda ki elemanları gezer.
        fread(&k1, sizeof(magaza), 1, fp);
        
        if(k1.magaza_id == NULL){                       //Adres alanı boşsa devam eder. 
            continue;
        }
        else{
            if(k1.sonraki != -1){                       //Adres alanında ki kayıdın gösterdiği başka bir kayıt varsa..
                int a = k1.sonraki;                     //Yer tutucu.
                k1.sonraki = ftell(fp_gecici); 
                fseek(fp, i*sizeof(magaza), SEEK_SET);
                fwrite(&k1, sizeof(magaza), 1, fp);
                        
                while(1){                              //Bağlı listenin sonuna kadar döner.
                    k1.sonraki = a;
                    
                    fseek(fp_chains, k1.sonraki, SEEK_SET);
                    fread(&k1, sizeof(magaza), 1, fp_chains);                    
                   
                    if(k1.sonraki == -1){               //Bağlı listenin sonuna ulaşılmışsa döngü sonlanır.
                        fwrite(&k1,sizeof(magaza), 1, fp_gecici); //Chains dosyasında ki kayit geçici dosyaya yazılır.
                        break;
                    }
                    else{                               //Bağlı listenin sonuna ulaşılmamışsa döngü devam eder.
                        a = k1.sonraki;
                        k1.sonraki = ftell(fp_gecici)+sizeof(magaza); 
                        fwrite(&k1,sizeof(magaza), 1, fp_gecici);
                    }
                }
                
            }
        }
    }
    remove("chains.bin");
    rename("gecici.bin","chains.bin");                  //Chains dosyası silinir.Geçici dosyası chains adını alır.
    fclose(fp_gecici);
    fclose(fp);
}

int arama()
{
    printf("********** Kayit Arama **********\n");
    FILE *fp = fopen("hash.bin", "rb");
    magaza k1;  //Okunucak kayit.
    char isim[20]; //Aranılıcak kaydın ismi.
    int hash;
    
    printf("Mağaza ismi:  ");
    scanf("%s", isim);
    
    hash = hash_f(isim);                                //Aranıcak kaydın isimini hash fonksiyonuna gönderiyoruz.
    fseek(fp, hash*sizeof(magaza), SEEK_SET);           //Hash fonksiyonun döndürdüğü değere konumlanıyoruz.
    fread(&k1, sizeof(magaza), 1, fp);
    
    if(!strcmp(k1.magaza_isim, isim)){ 
        printf("Magaza_ismi: %s  sehir:  %s  gelir:  %d  gider: %d  magaza_id: %d\n",k1.magaza_isim, k1.sehir, k1.gelir, k1.gider, k1.magaza_id);
    } 
    else if(k1.magaza_id == NULL || k1.sonraki == -1){ //Adres alanında kayıt yoksa veya O kayıda başka bir kayıt bağlı değilse.
        printf("Kayit Bulunamadi!!!\n");
    }
    else{                                                //İsimler aynı değilse chains dosyasına bakılır.
        FILE *fp_chains = fopen("chains.bin","r");
        int b = 0;           //Kontrol değişkeni.
        
        do{                                              //Zincirin elemanlarına bakar.
            fseek(fp_chains, k1.sonraki, SEEK_SET);
            fread(&k1, sizeof(magaza), 1, fp_chains);
            if(!strcmp(k1.magaza_isim, isim)){           //Aranılan kayıt bulunursa döngü kırılır.
                b = 1;
                break;
            }
        }while(k1.sonraki != -1 && k1.magaza_id != NULL);       
        
        if(b){
            printf("Magaza_ismi: %s  sehir:  %s  gelir:  %d  gider: %d  magaza_id: %d\n",k1.magaza_isim, k1.sehir, k1.gelir, k1.gider, k1.magaza_id);
        }
        else{
            printf("Kayit Bulunamadi!!!\n");
        }
        fclose(fp_chains);   
    }
    fclose(fp);
}

void guncelleme()
{
    printf("********** Kayit Güncelleme **********\n");
    FILE *fp = fopen("hash.bin", "rb+");
    magaza k1;  //Okunucak kayit.
    char isim[20]; //Aranılıcak kaydın ismi.
    int hash;
    
    printf("Mağaza ismi:  ");
    scanf("%s", isim);
    
    hash = hash_f(isim);
    fseek(fp, hash*sizeof(magaza), SEEK_SET);
    fread(&k1, sizeof(magaza), 1, fp);
    
    
    if(!strcmp(k1.magaza_isim, isim)){                 //İsimler aynıysa..
        printf("Yeni Sehir isimini giriniz:  ");
        scanf("%s",k1.sehir);
    
        printf("Yeni Geliri giriniz:  ");
        scanf("%d",&k1.gelir);
    
        printf("Yeni Gideri giriniz:  ");
        scanf("%d",&k1.gider);
        
        fseek(fp, hash*sizeof(magaza), SEEK_SET);
        fwrite(&k1, sizeof(magaza), 1, fp);        
    } 
    else if(k1.magaza_id == NULL || k1.sonraki == -1){ //Adres alanında kayıt yoksa veya O kayıda başka bir kayıt bağlı değilse.
        printf("Kayit Bulunamadi!!!\n");
    }
    else{                                                //İsimler aynı değilse chains dosyasına bakılır.
        FILE *fp_chains = fopen("chains.bin","rb+");
        int b = 0; //Kontrol değişkeni.
        
        do{                                              //Zincirin elemanlarına bakar.
            fseek(fp_chains, k1.sonraki, SEEK_SET);
            fread(&k1, sizeof(magaza), 1, fp_chains);
            if(!strcmp(k1.magaza_isim, isim)){
                b = 1;
                break;
            }
        }while(k1.sonraki != -1 && k1.magaza_id != NULL);
        
        if(b){                                          //Güncellenicek kayıt bulunursa döngü kırılır.
            printf("Yeni Sehir isimini giriniz:  ");
            scanf("%s",k1.sehir);
    
            printf("Yeni Geliri giriniz:  ");
            scanf("%d",&k1.gelir);
    
            printf("Yeni Gideri giriniz:  ");
            scanf("%d",&k1.gider);
            
            fseek(fp_chains, ftell(fp_chains)-sizeof(magaza), SEEK_SET);
            fwrite(&k1, sizeof(magaza), 1, fp_chains);
        }
        else
            printf("Kayit Bulunamadi!!!\n");
        
        fclose(fp_chains);
    }
  
    fclose(fp);
}
void hash_listele()                                     //Hash listesini ve çakışan kayıtları yazdırır.
{
    printf("********** Hash Tablosu **********\n");
    FILE *fp = fopen("hash.bin", "rb");
    FILE *fp_chains = fopen("chains.bin", "rb");
    magaza k1;
  
    for(int i = 0; i<table_size; i++){
        fread(&k1, sizeof(magaza), 1, fp);
        
        if(k1.magaza_id == NULL){
            printf("%d) \n",i);
        }
        else{
            printf("%d) %s", i, k1.magaza_isim);
            if(k1.sonraki != -1){
                do{
                    fseek(fp_chains, k1.sonraki, SEEK_SET);
                    fread(&k1, sizeof(magaza), 1, fp_chains);
                    printf(" -> %s",k1.magaza_isim);
                }while(k1.sonraki != -1);         
            }
            printf("\n");
        }
    }
    printf("*************************************\n");
    fclose(fp_chains);
    fclose(fp);
}

void menu()
{
    int i, i2;
    while(1){
        fflush(stdin);
        printf("********** Ana Menu **********\n");
        printf("1. Kullanıcı girisi\n");
        printf("2. Yönetici girisi\n");
        printf("3. Cikis\n");
        printf("-->  ");
        
        if(scanf("%d", &i)){
            if(i == 2){
                while(1){
                    fflush(stdin);
                    printf("********** Yoneciti Menusu ***********\n");      
                    printf("1. Kayit ekleme\n");
                    printf("2. Kayit silme\n");
                    printf("3. Kayit güncelleme\n");
                    printf("4. Kayitlarin listele\n");
                    printf("5. Geri\n");
                    printf("-->  ");
                    if(scanf("%d", &i2)){
                        if(i2 == 1){
                            kayit_ekleme();
                            continue;
                        }
                        else if(i2 == 2){
                            sil();
                            continue;
                        }
                        else if(i2 == 3){
                            guncelleme();
                            continue;
                        }
                        else if(i2 == 4){
                            hash_listele();
                            continue;
                        }
                        else if(i2 == 9){
                            e_liste_listele();
                            continue;
                        }
                        else if(i2 == 5){
                            break;
                        }
                        else{
                           printf("Lutfen 1-6 arasi deger giriniz...\n"); 
                           continue;
                        }
                    }
                    else{
                        printf("Lutfen 1-6 arasi deger giriniz...\n");                
                    }
                }
            }
            else if (i == 1){
                while(1){
                    printf("********** Kullanıcı Menusu ***********\n");
                    printf("1. Kayit arama\n");
                    printf("2. Geri\n");
                    printf("-->  ");
                    scanf("%d", &i2);

                    if(i2 == 1){
                        arama();
                        continue;
                    }
                    else if(i2 == 2){
                        break;
                    }
                    else{
                        printf("Lutfen 1-2 arasi deger giriniz...\n"); 
                        continue;
                    }
                }
            }
            else if(i == 3){
                return;
            }
            else{
                printf("Lutfen 1-3 arasi deger girin...\n");
                continue;
            }        
        }
        else{
            printf("Lutfen 1-3 arasi deger girin...\n");    
        }

    }
}

void e_liste_listele(){
    e_liste *x = liste_basi;
    while(x != NULL){
        printf("%d\t",x->adres);
        x = x->ileri;
    }
    printf("\n");
}

int dosya_varmi(char *file){                //Dosya varsa sıfır yoksa bir döner.
    FILE *fp;
    if ((fp = fopen(file ,"rb")) == NULL){
        fclose(fp);
        return 0;
    }
    return 1;
}

int main ()
{ 
    FILE *fp,*fp_chains;
    
    if(!dosya_varmi("hash.bin")){                     //Dosya varsa yeniden oluşturmaz.
        if((fp = fopen("hash.bin","wb")) == NULL){ 
            printf("Dosya oluşturulamadı!!");
            exit(1); 
        }
        ftruncate(fileno(fp), table_size * sizeof(magaza)); //Dosyayı belirlediğimiz boyut kadar açan fonksiyon.
        fclose(fp);                                                 
    }
         
    if(!dosya_varmi("chains.bin")){                  //Dosya varsa yeniden oluşturmaz.
        if((fp_chains = fopen("chains.bin","wb")) == NULL){
            printf("Dosya oluşturulamadı!!");
            exit(1);
        }
        fclose(fp_chains);
    }
    
    menu();
    silinmisleri_at();                              //Programdan çıktıktan sonra chains dosyasında ki silnmiş kayıtlar atılır.
    
    return 0;
}
