#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct fat_BS
{
	unsigned char bootjmp[3];
	unsigned char oem_name[8];
	unsigned short bytes_per_sector;
	unsigned char sectors_per_cluster;
	unsigned short reserved_sector_count;
	unsigned char table_count;
	unsigned short root_entry_count;
	unsigned short total_sectors_16;
	unsigned char media_type;
	unsigned short table_size_16;
	unsigned short sectors_per_track;
	unsigned short head_side_count;
	unsigned int hidden_sector_count;
	unsigned int total_sectors_32;
 
	unsigned char extended_section[54];
 
}__attribute__((packed)) fat_BS_t;

typedef struct fat_std
{
	unsigned char filename[8];
	unsigned char extension[3];
	unsigned char file_attributes;
	unsigned char reserved;
	unsigned char creation_time;
	unsigned short file_time;
	unsigned short file_date;
	unsigned short last_acess_date;
	unsigned short high_16;
	unsigned short last_modification_time;
	unsigned short last_modification_date;
	unsigned short low_16;
	unsigned int file_size;

}__attribute__((packed)) fat_std_t;

int main(){
	

    	FILE *fp;
    	fat_BS_t boot_record;
	fat_std_t standard_directory;

	fp = fopen("./floppyfat2.img", "rb");
	if(fp == NULL){
		printf("ERRO IMAGEM");
		exit(0);
	}
    
    	fseek(fp, 0, SEEK_SET); // Início do boot record
    	fread(&boot_record, sizeof(fat_BS_t), 1, fp); // Lê do boot record todo

    	printf("Bytes por setor (bytes_per_sector): %hd\n", boot_record.bytes_per_sector);
    	printf("Setores por cluster (sectors_per_cluster): %d \n", (int)boot_record.sectors_per_cluster);
    	printf("Numero de  FATs (table_count): %d\n", (int)boot_record.table_count);
    	printf("Setores por cluster: %hu\n", boot_record.bytes_per_sector);
    	printf("Tamanho fat (setores): %hu\n", boot_record.table_size_16);
    	printf("Setores por trilha: %hu\n", boot_record.sectors_per_track);

	//tamanho do diretório raiz
	unsigned short root_dir_sectors = ((boot_record.root_entry_count * 32) + (boot_record.bytes_per_sector -1)) / boot_record.bytes_per_sector;
	//primeiro setor de dados
	unsigned short first_data_sector = boot_record.reserved_sector_count + (boot_record.table_count * boot_record.table_size_16) + root_dir_sectors;
    	//primeiro setor do diretório raiz
	unsigned short first_root_dir_sector = first_data_sector - root_dir_sectors;

	printf("\nSetor onde a FAT se inicia: %hu\n", boot_record.reserved_sector_count);
	printf("Setor onde o diretorio raiz se inicia: %hu\n", first_root_dir_sector);
	printf("Setor onde o diretorio de dados se inicia: %hu\n", first_data_sector);
	
	//endereço de onde começa o diretório raiz
	unsigned int aux = (boot_record.reserved_sector_count + (int)boot_record.table_count * boot_record.table_size_16) * boot_record.bytes_per_sector;
	
	int x = boot_record.root_entry_count;
	while(x > 0){
		x -= 1;
		fseek(fp, aux, SEEK_SET);
		fread(&standard_directory, sizeof(fat_std_t), 1, fp);
		if (standard_directory.file_attributes == '\x10' || standard_directory.file_attributes == '\x20'){ //ou é diretório ou é arquivo
			if ((int)standard_directory.filename[0] != 229){  //ou foi apagado
				printf("\nnome: ");
				for (int i = 0; i < sizeof(standard_directory.filename); i++){
					printf("%c",standard_directory.filename[i]);
				}
				printf("\nextensao: ");
				for (int i = 0; i< sizeof(standard_directory.extension); i++){
					printf("%c", standard_directory.extension[i]);
				}
				printf("\n");
				printf("atributo: %x \n", (int)standard_directory.file_attributes);
				printf("primeiro cluster: %d \n", (int)standard_directory.low_16);
				printf("tamanho: %d\n\n", standard_directory.file_size);

				int cluster = ((standard_directory.low_16 - 2) * boot_record.sectors_per_cluster) + first_data_sector;
				fseek(fp, cluster*boot_record.bytes_per_sector, SEEK_SET);

				int val = (int)standard_directory.file_size;
				char buffer[val];

				fread(buffer,standard_directory.file_size, 1, fp);
				buffer[val] = '\0';
				printf("conteudo: %s\n", buffer);

			}
		}
		aux += 32;			
	}

	return 0;
}
