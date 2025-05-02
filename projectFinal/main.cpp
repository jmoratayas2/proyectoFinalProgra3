#include <iostream>
#include <cstdlib>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <direct.h> // _getcwd
#include <windows.h>
using namespace std;
using json = nlohmann::json;
const string API_KEY = "AIzaSyDjUcy9IZ8lnzbpRnltVrU7rS4_bNqunUc"; //variable global, es el api key para acceder al api de google.
string logUsuario = "";

// Estructura para verificar el lenguaje
struct Lenguajes {
    string lenguaje;
    string nombre;
};

// Estructura para verificar el lenguaje
struct Usuarios {
    string nombre1;
    string apellido1;
    string carne;
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

//Funcion para agregar la carpeta de cada usuario
void existeCarpetaUsuario(string usuario){
    string nuevaRuta = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        nuevaRuta = string(cwd)+"\\files\\users\\"+usuario;
    }

    size_t pos = 0;
    string subruta = "";
    while ((pos = nuevaRuta.find('\\', pos)) != string::npos) {
        subruta = nuevaRuta.substr(0, pos);
        _mkdir(subruta.c_str());  // Crear cada nivel de carpeta
        pos++;
    }

    // Crear la carpeta final
    if (_mkdir(nuevaRuta.c_str()) == 0) {
    } else {
    }
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

/*Se crea un struct para almacenar las traducciones y sus funciones adaptables*/
struct Traducciones {
    string espanol;
    string ingles;
    string italiano;
    string frances;
    string aleman;

    void guardar(ofstream& out) const {
        guardarCadena(out, espanol);
        guardarCadena(out, ingles);
        guardarCadena(out, italiano);
        guardarCadena(out, frances);
        guardarCadena(out, aleman);
    }

     void cargar(ifstream& in) {
        espanol = cargarCadena(in);
        ingles = cargarCadena(in);
        italiano = cargarCadena(in);
        frances = cargarCadena(in);
        aleman = cargarCadena(in);
    }

    private:
    static void guardarCadena(ofstream& out, const string& texto) {
        uint32_t longitud = texto.size();
        out.write(reinterpret_cast<char*>(&longitud), sizeof(longitud));
        out.write(texto.c_str(), longitud);
    }

    static string cargarCadena(ifstream& in) {
        uint32_t longitud;
        in.read(reinterpret_cast<char*>(&longitud), sizeof(longitud));
        string texto(longitud, '\0');
        in.read(&texto[0], longitud);
        return texto;
    }
};

//Funcion para realizar la traduccion, a los 4 idiomas.
void translate(){
    string text;
    cout<<"Ingresa el texto en espanol a traducir"<<endl;
    cin>>text;
    Traducciones t;
    t.espanol = text;

    for (const auto& idioma : lenguajes) {
        string response = translateText(text, idioma.lenguaje);
        string translatedText = extractTranslation(response);

        if (!translatedText.empty()) {
            cout << "Texto traducido al "<<idioma.nombre <<": "<< translatedText << endl;
            if (idioma.nombre == "Ingles")
                t.ingles = translatedText;
            else if (idioma.nombre == "Italiano")
                t.italiano = translatedText;
            else if (idioma.nombre == "Frances")
                t.frances = translatedText;
            else if (idioma.nombre == "Aleman")
                t.aleman = translatedText;
        } else {
            cerr << "Error al procesar la respuesta JSON." << endl;
        }
    }

    //Para el historial
    if(existeArchivoONo("historial.bin")){
        string rutaArchivo = "";
        char cwd[FILENAME_MAX];
        if (_getcwd(cwd, sizeof(cwd))) {
            rutaArchivo = string(cwd)+"\\files\\historial.bin";
        }
        ofstream file(rutaArchivo, ios::binary | ios::app);
        if (file.is_open()) {
            t.guardar(file);
            file.close();
//            cout << "Traducción guardada correctamente en el archivo binario." << endl;
        } else {
//            cerr << "No se pudo abrir el archivo binario para escritura." << endl;
        }
    } else {
//        cerr << "Hubo un error al almacenar las traducciones" << endl;
    }

    //Para el historialUsuario
    if(existeArchivoONo("users\\"+logUsuario+"\\userHistorial.bin")){
        string rutaArchivo = "";
        char cwd[FILENAME_MAX];
        if (_getcwd(cwd, sizeof(cwd))) {
            rutaArchivo = string(cwd)+"\\files\\users\\"+logUsuario+"\\userHistorial.bin";
        }
        ofstream file(rutaArchivo, ios::binary | ios::app);
        if (file.is_open()) {
            t.guardar(file);
            file.close();
            cout << "Traducción guardada correctamente en el archivo binario." << endl;
        } else {
            cerr << "No se pudo abrir el archivo binario para escritura." << endl;
        }
    } else {
        cerr << "Hubo un error al almacenar las traducciones" << endl;
    }
}


//Funcion para cargar el archivo del historial completo.
void leerHistorial() {
     if(existeArchivoONo("historial.bin")){
            string rutaArchivo = "";
            char cwd[FILENAME_MAX];
            if (_getcwd(cwd, sizeof(cwd))) {
                rutaArchivo = string(cwd)+"\\files\\historial.bin";
            }
            ifstream file(rutaArchivo, ios::binary);
            if (!file.is_open()) {
                cerr << "No se pudo abrir el archivo para lectura." << endl;
                return;
            }

            while (file.peek() != EOF) {
                Traducciones t;
                t.cargar(file);
                cout << t.espanol << " | " << t.ingles << " | " << t.italiano << " | "
                    << t.frances << " | " << t.aleman << endl;
            }

            file.close();
     }else{
         cerr << "No existe ningun historial" << endl;
     }
}


//Funcion para mostrar el segundo menu
void menu2(){
    cout<<"Ingresa a una opcion"<<endl;
    cout<<"1. Traducir."<<endl;
    cout<<"2. Ver mi historial."<<endl;
    cout<<"3. Salir de mi usuario."<<endl;
}


//Segunda vista al momento de que el usuario ingresa
void panelPrincipal(){
    int opcion = 0;
    do{
        menu2();
        cin >> opcion;
        switch(opcion){
            case 1:
                translate();
            break;
            case 2:
            break;
            case 3:
                cout<<"Regresando a menu principal."<<endl;
                logUsuario = "";
                system("cls");
                return;
            break;
            default:
                cout<<"Opcion no valida"<<endl;
            break;
        }
    }while(opcion != 3);
}


//Funcion para mostrar el primer menu
void menu1(){
    cout<<"Ingresa a una opcion"<<endl;
    cout<<"1. Ingresar."<<endl;
    cout<<"2. Registrarte."<<endl;
    cout<<"3. Salir."<<endl;
}

//funcion para agregar un nuevo usuario
void agregarUsuario(){
    system("cls");
    Usuarios usuario;
    string usuarioIngreso = "";
    cout<<"Ingresa tu primer nombre:"<<endl;
    cin >> usuario.nombre1;
    cout<<"Ingresa tu primer apellido:"<<endl;
    cin >> usuario.apellido1;
    cout<<"Ingresa carne completo (sin guiones, ni espacios):"<<endl;
    cin >> usuario.carne;
    usuarioIngreso = usuario.nombre1+"_"+usuario.apellido1+"_"+usuario.carne;
    cout<<"Tu usuario es: "<<usuarioIngreso<<endl;
    logUsuario = usuarioIngreso;
    existeCarpetaUsuario(usuarioIngreso);
}

//Verificamos si existe el directorio, para ver si existe el usuario
bool existeDirectorio(const string& ruta) {
    return _access(ruta.c_str(), 0) == 0;
}

//funcion para ingresar usuario
void ingresoUsuario(){
    system("cls");
    string usuario = "";
    cout<<"Ingresa tu usuario:"<<endl;
    cin>>usuario;
    string rutaCarpeta = "";
    //Obtenemos la ruta del directorio actual, por si llega a cambiar la ubicacion (_getcwd)
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        rutaCarpeta = string(cwd)+"\\files\\users\\"+usuario;
    }
    if(existeDirectorio(rutaCarpeta)){
        cout<<"Bienveido a tu traductor"<<endl;
        SetConsoleTitle(usuario.c_str());
        logUsuario = usuario;
        panelPrincipal();
    }else {
        cout<<"No cuentas con usuario, deseas crear una cuenta?"<<endl;
        cout<<"1. Si"<<endl;
        cout<<"2. No"<<endl;
        int opcion = 0;
        cin>>opcion;
        if(opcion == 1){
            system("cls");
            agregarUsuario();
        }else{
            system("cls");
        };
    }
}

//Iniciamos con el menu del proyecto
void iniciarProyecto(){
    int opcion = 0;
    system("cls");
    do{
        SetConsoleTitle("Traductor UMG");
        menu1();
        cin >> opcion;
        switch(opcion){
            case 1:
                ingresoUsuario();
            break;
            case 2:
                agregarUsuario();
            break;
            case 3:
                cout<<"Saliendo..."<<endl;
            break;
            default:
                cout<<"Opcion no valida"<<endl;
            break;
        }
    }while(opcion != 3);
}

int main() {
    iniciarProyecto();
    return 0;
}

