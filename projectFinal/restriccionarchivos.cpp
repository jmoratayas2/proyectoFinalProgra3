
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> // para usar  system()
using namespace std;


bool autenticar(string user, string pass) {  // Función de autenticación
    return user == "admin" && pass == "1234";
}


void protegerArchivo() {// Función para proteger el archivo con attrib
    system("attrib +r +h traducciones.txt");
}

// 🔓 Función para desproteger el archivo
void desprotegerArchivo() {
    system("attrib -r -h traducciones.txt");
}

void guardarTraduccion(const string& palabra, const string& traduccion) { //  Función para guardar traducción

    desprotegerArchivo(); // primero quitar protección

    ofstream archivo("traducciones.txt", ios::app);
    if (archivo.is_open()) {
        archivo << palabra << " -> " << traduccion << endl;
        archivo.close();
        cout << "Traducción guardada.\n";
    } else {
        cout << "No se pudo guardar la traducción.\n";
    }

    protegerArchivo(); // volver a proteger
}

int main() {
    string usuario, clave;
    cout << "Usuario: ";
    cin >> usuario;
    cout << "Contraseña: ";
    cin >> clave;

    if (!autenticar(usuario, clave)) {
        cout << "Acceso denegado.\n";
        return 0;
    }

    cout << "Bienvenido al Traductor.\n";
    string palabra, traduccion;

    cout << "Ingresa la palabra a traducir: ";
    cin >> palabra;
    cout << "Traducción: ";
    cin >> traduccion;

    guardarTraduccion(palabra, traduccion);

    return 0;
}
