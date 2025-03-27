#include <iostream>
#include <fstream>

using namespace std;

// Funci�n para crear un archivo dentro de la carpeta proyectoFinalProgra3
void crearArchivotxt()
{
    string folderName = "proyectoFinalProgra3";  // Carpeta donde se crear� el archivo
    string filePath = folderName + "C:\proyectoFinalProgra3\projectFinal";  // Ruta en donde se encuentra el archivo

    ofstream file(filePath);
    if (file.is_open())
    {
        file.close();
        cout << "El archivo se ha creado exitosamente en: " << filePath << endl;
    }
    else
    {
        cout << "Error al crear archivo." << endl;
    }
}

int main()
{
    cout << "Bienvenido al traductor UMG - Cambio Fabiola" << endl;

    // Llamar a la funci�n para crear el archivo
    crearArchivotxt();

    return 0;
}

