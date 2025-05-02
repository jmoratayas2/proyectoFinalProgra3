#include <iostream>
#include <cstdlib>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <direct.h> // _getcwd
using namespace std;
using json = nlohmann::json;
const string API_KEY = "AIzaSyDjUcy9IZ8lnzbpRnltVrU7rS4_bNqunUc"; //variable global, es el api key para acceder al api de google.

// Estructura para verificar el lenguaje
struct Lenguajes {
    string lenguaje;
    string nombre;
};

// Estructura para el archivo binario
struct LenguajeBinario {
    char lenguaje[30];
    char nombre[30];
};

//Insertamos el listado de idiomas
Lenguajes lenguajes[] = {
    {"it", "Italiano"},
    {"en", "Ingles"},
    {"fr", "Frances"},
    {"de", "Aleman"}
};

// Función para manejar la respuesta de la API
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//Funcion para el consumo del api de google translate
string translateText(const string& text, const string& targetLanguage) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        string url = "https://translation.googleapis.com/language/translate/v2?key=" + API_KEY;

        string jsonBody = "{\"q\": \"" + text + "\", \"target\": \"" + targetLanguage + "\"}";

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLoption::CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLoption::CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLoption::CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLoption::CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLoption::CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLoption::CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {//Verificamos la respuesta del api
            cerr << "Error en la solicitud: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}

//Funcion para obtener como json la respuesta del api.
string extractTranslation(const string& jsonResponse) {
    json responseJson = json::parse(jsonResponse);
    return responseJson["data"]["translations"][0]["translatedText"];
}

//Funcion para realizar la traduccion, a los 4 idiomas.
void translate(){
    string text;
    cout<<"Ingresa el texto en espanol a traducir"<<endl;
    cin>>text;

    for (const auto& idioma : lenguajes) {
        string response = translateText(text, idioma.lenguaje);
        string translatedText = extractTranslation(response);

        if (!translatedText.empty()) {
            cout << "Texto traducido al "<<idioma.nombre <<": "<< translatedText << endl;
        } else {
            cerr << "Error al procesar la respuesta JSON." << endl;
        }
    }
}

//Funcion para verificar si el archivo existe o no.
bool existeArchivoONo(string nombreArchivo){
    string rutaArchivo = "";
    //Obtenemos la ruta del directorio actual, por si llega a cambiar la ubicacion (_getcwd)
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        rutaArchivo = string(cwd)+"\\files\\"+nombreArchivo;
    }

    //Se verifica si existe el archivo
    ifstream archivoExistente(rutaArchivo, ios::binary);
    if (archivoExistente.is_open()) {
        //Si el archivo existe ya no se procede a crearse
        archivoExistente.close();
        return true;
    }
    //Sino existe, el archivo se generara
    ofstream archivo(rutaArchivo, ios::binary);
    if (archivo.is_open()) {
        archivo.close();
        //Se crea el archivo exitosamente.
    } else {
        //No se pudo crear el archivo.
        return false;
    }
    return true;
}

/*
    Funciones para crear la encriptacion
*/
//Obtenemos el indice de las vocales
int obtenerIndiceVocal(char c) {
    // Pasar a minúscula
    unsigned char uc = static_cast<unsigned char>(c);

    switch (uc) {
        case 'a':return 1;
        case 'e':return 2;
        case 'i':return 3;
        case 'o':return 4;
        case 'u':return 5;
        case 160: return 1;
        case 130: return 2;
        case 161: return 3;
        case 162: return 4;
        case 163: return 5;
        default: return 0;
    }
}
//saber si es ñ
bool esEnie(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    return uc == 164 || uc == 165; // 164: ñ, 165: Ñ en tu consola/codificación
}

//saber si es mayuscula
bool esMayu(char c){
    unsigned char uc = static_cast<unsigned char>(c);
    if(uc == 165) return true;
    return false;
}

//Obtenemos el indice de las consonantes
int obtenerIndiceConsonante(char text) {
    text = tolower(text);

    if (!isalpha(text) || obtenerIndiceVocal(text) > 0) return 0;

    string consonantes = "bcdfghjklmnñpqrstvwxyz";

    for (size_t i = 0; i < consonantes.length(); ++i) {
        if (consonantes[i] == text) {
            if(i > 12) return i;
            return i+1;
        }
    }
    return 0;
}

//Funcion para encriptar el texto
string encriptarTexto(const string& texto, size_t pos = 0){
    if (pos >= texto.length()) return "";  // Caso base
    char c = texto[pos];
    string codificado = "";
    int idxV = obtenerIndiceVocal(c);
    if (idxV > 0) {
        codificado = "U" + to_string(idxV);
    } else if (islower(c)) {
        int idx = obtenerIndiceConsonante(c);
        if (idx > 0) codificado = "m" + to_string(idx);
        else codificado = c;
    } else if (isupper(c)) {
        int idx = obtenerIndiceConsonante(c);
        if (idx > 0) codificado = "g" + to_string(idx);
        else codificado = c;
    } else {
        if(esEnie(c)){
            codificado = esMayu(c) ? "g12" : "m12";
        } else {
            codificado = c;
        }
    }

    // Llamada recursiva para el resto de la cadena
    return codificado + encriptarTexto(texto, pos + 1);
}

int main() {
    return 0;
}

