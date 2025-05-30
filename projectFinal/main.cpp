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
string encriptarTexto(const string& texto, const string& clave, size_t pos = 0){
    if (pos >= texto.length()) return "";
    char c = texto[pos];
    string codificado = "";
    int idxV = obtenerIndiceVocal(c);
    if (idxV > 0) {
        codificado = string(1, clave[0]) + to_string(idxV);
    } else if (islower(c)) {
        int idx = obtenerIndiceConsonante(c);
        if (idx > 0) codificado = string(1,clave[1]) + to_string(idx);
        else codificado = string(1,c);
    } else if (isupper(c)) {
        int idx = obtenerIndiceConsonante(c);
        if (idx > 0) codificado = string(1,clave[2]) + to_string(idx);
        else codificado = string(1,c);
    } else {
        if(esEnie(c)){
            codificado = string(1, esMayu(c) ? clave[2] : clave[1]) +"12";
        } else {
            codificado = string(1,c);
        }
    }

    // Llamada recursiva para el resto de la cadena
    return codificado + encriptarTexto(texto, clave, pos + 1);
}

//Funcion para reemplazar la nueva clave.
void reemplazarPalabrasConNuevaClave(const string& usuario, const string& claveAnterior, const string& claveNueva){
    string ruta = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        ruta = string(cwd) + "\\files\\users\\" + usuario + "\\encriptados.enc";
    }

    ifstream entrada(ruta);
    if (!entrada.is_open()) {
        cerr << "No se pudo abrir el archivo encriptado." << endl;
        return;
    }
    vector<string> nuevasLineas;
    string linea;

    while (getline(entrada, linea)) {
        for (char& c : linea) {
            if (c == claveAnterior[0]) c = claveNueva[0];
            else if (c == claveAnterior[1]) c = claveNueva[1];
            else if (c == claveAnterior[2]) c = claveNueva[2];
        }
        nuevasLineas.push_back(linea);
    }

    entrada.close();

    ofstream salida(ruta, ios::trunc);
    if (salida.is_open()) {
        for (const string& l : nuevasLineas) {
            salida << l << "\n";
        }
        salida.close();
        cout << "Palabras encriptadas con nueva clave." << endl;
    }
}

/*Se crea un struct para almacenar las traducciones y sus funciones adaptables*/
struct Traducciones {
    string espanol;
    string ingles;
    string italiano;
    string frances;
    string aleman;
    int contador = 1;

    void guardar(ofstream& out) const {
        guardarCadena(out, espanol);
        guardarCadena(out, ingles);
        guardarCadena(out, italiano);
        guardarCadena(out, frances);
        guardarCadena(out, aleman);
        out.write(reinterpret_cast<const char*>(&contador), sizeof(contador));
    }

     void cargar(ifstream& in) {
        espanol = cargarCadena(in);
        ingles = cargarCadena(in);
        italiano = cargarCadena(in);
        frances = cargarCadena(in);
        aleman = cargarCadena(in);
        in.read(reinterpret_cast<char*>(&contador), sizeof(contador));
    }

    private:
    static void guardarCadena(ofstream& out, const string& texto) {
        uint32_t longitud = texto.size();
        out.write(reinterpret_cast<const char*>(&longitud), sizeof(longitud));
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

//Estructura para el Arbol AVL
struct NodoAvl {
    Traducciones dato;
    NodoAvl* izquierdo;
    NodoAvl* derecho;
    int altura;
    NodoAvl(const Traducciones& t):dato(t), izquierdo(nullptr), derecho(nullptr), altura(1){};
};

//determinamos la altura del arbol
int altura(NodoAvl* n){
    return n ? n->altura : 0;
}

//maximo de la altura del arbol
int max(int a, int b){
    return (a>b) ? a : b;
}

//Funcion para la rotacion hacia la derecha
NodoAvl* rotacionDerecha(NodoAvl* y){
    NodoAvl* x = y->izquierdo;
    NodoAvl* T2 = x->derecho;

    x->derecho = y;
    y->izquierdo = T2;

    y->altura = max(altura(y->izquierdo), altura(y->derecho))+1;
    x->altura = max(altura(x->izquierdo), altura(x->derecho))+1;
    return x;
}

//Funcion para la rotacion hacia la izquierda
NodoAvl* rotacionIzquierda(NodoAvl* x) {
    NodoAvl* y = x->derecho;
    NodoAvl* T2 = y->izquierdo;

    y->izquierdo = x;
    x->derecho = T2;

    x->altura = max(altura(x->izquierdo), altura(x->derecho)) + 1;
    y->altura = max(altura(y->izquierdo), altura(y->derecho)) + 1;

    return y;
}

//Funcion para definir el balance
int balance(NodoAvl* n) {
    return n ? altura(n->izquierdo) - altura(n->derecho) : 0;
}

//Funcion para insertar al arbol AVL
NodoAvl* insertarAvl(NodoAvl* nodo, Traducciones t){
    if(!nodo) return new NodoAvl(t);

    if(t.espanol < nodo->dato.espanol)
        nodo->izquierdo = insertarAvl(nodo->izquierdo, t);
    else if(t.espanol > nodo->dato.espanol)
        nodo->derecho = insertarAvl(nodo->derecho, t);
    else
        return nodo;

    nodo->altura = 1 + max(altura(nodo->izquierdo), altura(nodo->derecho));
    int balanceFactor = balance(nodo);

    //Hacer la rotacion segun el balance
    if(balanceFactor > 1 && t.espanol < nodo->izquierdo->dato.espanol)
        return rotacionDerecha(nodo);
    if(balanceFactor < -1 && t.espanol > nodo->derecho->dato.espanol)
        return rotacionIzquierda(nodo);
    if(balanceFactor > 1 && t.espanol > nodo->izquierdo->dato.espanol){
        nodo->izquierdo = rotacionIzquierda(nodo->izquierdo);
        return rotacionDerecha(nodo);
    }
    if(balanceFactor < -1 && t.espanol < nodo->derecho->dato.espanol){
        nodo->derecho = rotacionDerecha(nodo->derecho);
        return rotacionIzquierda(nodo);
    }
    return nodo;
}

//Funcion para cargar el historial general al arbol
NodoAvl* cargarHistorialAvl(){
    NodoAvl* raiz = nullptr;
    if(existeArchivoONo("historial.bin")){
        string rutaArchivo = "";
        char cwd[FILENAME_MAX];
        if (_getcwd(cwd, sizeof(cwd))) {
            rutaArchivo = string(cwd) + "\\files\\historial.bin";
        }

        ifstream file(rutaArchivo, ios::binary);
        if (!file.is_open()) {
            cerr << "No se pudo abrir historial.bin para lectura" << endl;
            return nullptr;
        }

        while (file.peek() != EOF) {
            Traducciones t;
            t.cargar(file);
            raiz = insertarAvl(raiz, t);
        }

        file.close();
    }
    return raiz;
}

//Funcion para cargar el historial por usuario
NodoAvl* cargarHistorialUsuarioAvl(const string& usuario){
    string rutaArchivo = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        rutaArchivo = string(cwd)+"\\files\\users\\"+usuario+"\\userHistorial.bin";
    }

    ifstream file(rutaArchivo, ios::binary);
    if(!file.is_open()) return nullptr;

    NodoAvl* raiz = nullptr;
    while(file.peek() != EOF){
        Traducciones t;
        t.cargar(file);
        raiz = insertarAvl(raiz, t);
    }
    file.close();
    return raiz;
}

//Funcion para buscar en el arbol Avl
Traducciones* buscarAlAvl(NodoAvl* nodo, const string& espanol){
    if(!nodo) return nullptr;

    if(espanol == nodo->dato.espanol){
        return &(nodo->dato);
    } else if(espanol < nodo->dato.espanol){
        return buscarAlAvl(nodo->izquierdo, espanol);
    } else {
        return buscarAlAvl(nodo->derecho, espanol);
    }
}

//Funcion para recorrer el arbon y guardar todos los nodos
void guardarAvlEnArchivo(NodoAvl* nodo, ofstream& archivo){
    if(!nodo) return;

    guardarAvlEnArchivo(nodo->izquierdo, archivo);
    nodo->dato.guardar(archivo);
    guardarAvlEnArchivo(nodo->derecho, archivo);
}

//Sobre escribir el historial desde el AVL
void sobreEscribirDesdeAvl(NodoAvl* raiz){
    string rutaArchivo = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        rutaArchivo = string(cwd) + "\\files\\historial.bin";
    }

    ofstream archivo(rutaArchivo, ios::binary | ios::trunc);
    if(!archivo.is_open()){
        return; //No se puede abrir el archivo
    }

    guardarAvlEnArchivo(raiz, archivo);
    archivo.close();
}

void sobreEscribirUsuarioDesdeAvl(const string& usuario, NodoAvl* raiz){
    string rutaArchivo = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        rutaArchivo = string(cwd)+"\\files\\users\\"+usuario+"\\userHistorial.bin";
    }

    ofstream archivo(rutaArchivo, ios::binary | ios::trunc);
    if(!archivo.is_open()) return;
    guardarAvlEnArchivo(raiz, archivo);
    archivo.close();
}

//Funcion para recorrer al arbol para el contador
void recorrerAvl(NodoAvl* nodo, vector<Traducciones>& lista){
    if(!nodo) return;
    recorrerAvl(nodo->izquierdo, lista);
    lista.push_back(nodo->dato);
    recorrerAvl(nodo->derecho, lista);
}

//Funcion para ver las palabras mas buscadas por usuario
void verPalabrasConcurrentes(NodoAvl* raizUsuario){
    vector<Traducciones> lista;
    recorrerAvl(raizUsuario, lista);
    sort(lista.begin(), lista.end(), [](const Traducciones& a, const Traducciones& b){
         return a.contador > b.contador;
         });
         for(const auto& t: lista){
            cout <<"- "<<t.espanol<<" ("<<t.contador<<" veces)"<<endl;
         }
}

//Funcion para preguntar en que idioma quiere reproducirlo.
void menuTraducir(){
    cout<<"Que idioma quiere hacer la traduccion?"<<endl;
    cout<<"1. Ingles"<<endl;
    cout<<"2. Italiano"<<endl;
    cout<<"3. Frances"<<endl;
    cout<<"4. Aleman"<<endl;
}

void generarTraduccion(string texto, string idioma){
    string comando = "cmd.exe /C \"\"C:\\Program Files (x86)\\eSpeak\\command_line\\espeak.exe\" -v " + idioma + "+male" + " -s 150 -p 70 \"" + texto + "\"\"";
    // Ejecutar el comando
    system(comando.c_str());
}

//Funcion para ver el idioma a traducir
void seleccionTraduccion(Traducciones t){
    int opcion = 0;
    string texto;
    string idioma;
    menuTraducir();
    cin>>opcion;
    switch(opcion) {
        case 1: idioma = "en"; texto = t.ingles; cout<<"Traducido al ingles: "<<texto; break;  // Inglés
        case 2: idioma = "it"; texto = t.italiano; cout<<"Traducido al italiano: "<<texto; break;  // Italiano
        case 3: idioma = "fr"; texto = t.frances; cout<<"Traducido al frances: "<<texto; break;  // Francés
        case 4: idioma = "de"; texto = t.aleman; cout<<"Traducido al aleman: "<<texto; break;  // Alemán
        default:
            cout << "Opción no válida, se usará el idioma por defecto (Inglés).\n";
            idioma = "en"; texto = t.ingles;  // Default to English
            break;
    }
    int opcionTraduccion = 0;
    do{
        cout<<"\nQuieres escuchar la traduccion?"<<endl;
        cout<<"1. Si"<<endl;
        cout<<"2. No"<<endl;
        cin>>opcionTraduccion;
        switch(opcionTraduccion){
            case 1:
                generarTraduccion(texto, idioma);
            break;
            case 2:
                cout<<"Hasta luego"<<endl;
                system("cls");
            break;
            default:
                cout<<"Opcion no valida"<<endl;
            break;
        }
    }while(opcionTraduccion != 2);
}

//Obtener clave del usuario
string obtenerClave(const string& usuario){
    string rutaClave = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        rutaClave = string(cwd) + "\\files\\users\\" + usuario + "\\item.key";
    }

    ifstream file(rutaClave);
    if (!file.is_open()) {
        return "Umg"; // Si no se puede leer, retornamos uno por defecto
    }

    string clave;
    getline(file, clave);
    file.close();

    return clave;
}

//Funcion para validar la clave
bool claveValida(const string& clave){
    return clave.length() == 3 &&
           isupper(clave[0]) &&
           islower(clave[1]) &&
           islower(clave[2]);
}

//Funcion para cambiar la clave
void cambiarClave(const string& usuario) {
    int salir = 0;
    do{
        string nuevaClave;
        cout << "Ingrese la nueva clave (3 caracteres: Mayúscula + 2 minúsculas): ";
        cin >> nuevaClave;
        if (!claveValida(nuevaClave)) {
            cout << "Clave invalida, formato correcto: 'Abc'" << endl;
            salir = 0;
        } else{
            string claveAnterior = obtenerClave(usuario);
            reemplazarPalabrasConNuevaClave(usuario, claveAnterior, nuevaClave);
            string ruta = "";
            char cwd[FILENAME_MAX];
            if (_getcwd(cwd, sizeof(cwd))) {
                ruta = string(cwd) + "\\files\\users\\" + usuario + "\\item.key";
            }

            ofstream file(ruta);
            if (file.is_open()) {
                file << nuevaClave;
                file.close();
                cout << "Clave actualizada correctamente." << endl;
            }
            salir = 1;
        }
    }while(salir !=1);
}

//Guardar encriptado si la palabra no existe
void guardarEncriptado(const string& palabra, const string& usuario, NodoAvl* arbolUsuario){
     if (buscarAlAvl(arbolUsuario, palabra)) {
        return; //Si ya existe, no se guarda
    }
    string clave = obtenerClave(usuario);
    string palabraEncriptada = encriptarTexto(palabra, clave);

    string ruta = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        ruta = string(cwd) + "\\files\\users\\" + usuario + "\\encriptados.enc";
    }
    ofstream file(ruta, ios::app);
    if (file.is_open()) {
        file << palabraEncriptada << "\n";
        file.close();
    }
}

//Funcion para realizar la traduccion, a los 4 idiomas.
void translate(NodoAvl*& arbolHistorial, NodoAvl*& arbolHistorialUsuario){
    string text;
    cout<<"Ingresa el texto en espanol a traducir"<<endl;
    cin>>text;

    guardarEncriptado(text, logUsuario, arbolHistorialUsuario);
    Traducciones* global = buscarAlAvl(arbolHistorial, text);
    Traducciones* user = buscarAlAvl(arbolHistorialUsuario, text);

    Traducciones t;
    if(global && user){
        global->contador++;
        user->contador++;
        t = *global;
    }else if(global){
        global->contador++;
        t = *global;
        t.contador = 1;
        arbolHistorialUsuario = insertarAvl(arbolHistorialUsuario, t); // insertar en usuario
    }else if(user){
        user->contador++;
        t = *user;
        t.contador = 1;
        arbolHistorial = insertarAvl(arbolHistorial, t);
    } else {
        t.espanol = text;
        for (const auto& idioma : lenguajes) {
                string response = translateText(text, idioma.lenguaje);
                string translatedText = extractTranslation(response);

                if (!translatedText.empty()) {
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
            }
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
            }
        }
        arbolHistorial = insertarAvl(arbolHistorial, t);
        arbolHistorialUsuario = insertarAvl(arbolHistorialUsuario, t);
    }
    seleccionTraduccion(t);
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

//Funcion para crear archivo por defecto
void crearClavePorDefecto(const string& usuario){
    string ruta = "";
    char cwd[FILENAME_MAX];
    if (_getcwd(cwd, sizeof(cwd))) {
        ruta = string(cwd) + "\\files\\users\\" + usuario + "\\item.key";
    }

    ofstream file(ruta);
    if (file.is_open()) {
        file << "Umg";
        file.close();
    }
}

//Funcion para mostrar el segundo menu
void menu2(){
    cout<<"Ingresa a una opcion"<<endl;
    cout<<"1. Traducir."<<endl;
    cout<<"2. Ver mi historial."<<endl;
    cout<<"3. Cambiar clave."<<endl;
    cout<<"4. Salir de mi usuario."<<endl;
}

//Segunda vista al momento de que el usuario ingresa
void panelPrincipal(NodoAvl*& arbolHistorial, NodoAvl*& arbolHistorialUsuario){
    int opcion = 0;
    do{
        menu2();
        cin >> opcion;
        switch(opcion){
            case 1:
                translate(arbolHistorial, arbolHistorialUsuario);
            break;
            case 2:
                verPalabrasConcurrentes(arbolHistorialUsuario);
            break;
            case 3:
                cambiarClave(logUsuario);
            break;
            case 4:
                sobreEscribirUsuarioDesdeAvl(logUsuario, arbolHistorialUsuario);
                cout<<"Regresando a menu principal."<<endl;
                logUsuario = "";
                system("cls");
                return;
            break;
            default:
                cout<<"Opcion no valida"<<endl;
            break;
        }
    }while(opcion != 4);
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
    existeCarpetaUsuario(logUsuario);
    crearClavePorDefecto(logUsuario);
}

//Verificamos si existe el directorio, para ver si existe el usuario
bool existeDirectorio(const string& ruta) {
    return _access(ruta.c_str(), 0) == 0;
}

//funcion para ingresar usuario
void ingresoUsuario(NodoAvl*& arbolHistorial){
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
        logUsuario = usuario;
        cout<<"Bienvenido a tu traductor"<<endl;
        SetConsoleTitle(usuario.c_str());
        NodoAvl* arbolHistorialUsuario = cargarHistorialUsuarioAvl(logUsuario);
        panelPrincipal(arbolHistorial, arbolHistorialUsuario);
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
void iniciarProyecto(NodoAvl*& arbolHistorial){
    int opcion = 0;
    system("cls");
    do{
        SetConsoleTitle("Traductor UMG");
        menu1();
        cin >> opcion;
        switch(opcion){
            case 1:
                ingresoUsuario(arbolHistorial);
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
    NodoAvl* arbolHistorial = cargarHistorialAvl();
    iniciarProyecto(arbolHistorial);
    sobreEscribirDesdeAvl(arbolHistorial);//Actualiza al salir
    return 0;
}

