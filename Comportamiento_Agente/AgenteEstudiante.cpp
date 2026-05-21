#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo)
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false)
{
  nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const
{
  return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero &tablero)
{
  std::pair<int, int> mejor;
  nodosVisitados = 0;
  abortarBanda = false;
  inicioBusqueda = std::chrono::steady_clock::now();

  switch (modo)
  {
  case ModoJuego::ALEATORIO:
    return JuegaAleatorio(tablero);
    break;

  case ModoJuego::STATUS:
    Status(tablero, mejor);
    return mejor;
    break;

  case ModoJuego::MINIMAX:
    minimax(tablero, 0, profundidadMax, mejor);
    return mejor;
    break;

  case ModoJuego::INTELIGENTE:
    return JuegaInteligente(tablero);
    break;
  }

  return {-1, -1};
}

/**
 * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
 * @param padre Estado inicial del tablero.
 * @param hijo Estado resultante tras un movimiento.
 * @return Un par (fila, columna) con la posición de la nueva pieza.
 */
std::pair<int, int> SacarMovimiento(const Tablero &padre, const Tablero &hijo)
{
  for (int f = 0; f < padre.getFilas(); ++f)
    for (int c = 0; c < padre.getColumnas(); ++c)
      if (padre.getCelda(f, c) == 0 && hijo.getCelda(f, c) != 0)
        return {f, c};
  return {-1, -1};
}

/**
 * @brief Implementa un agente que juega de forma totalmente aleatoria.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida al azar.
 */
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero &tablero)
{

  // Calculo los tableros descendientes de tablero
  auto sucesores = tablero.getSucesores();

  // Si no tiene descendientes, paso el turno
  if (sucesores.empty())
    return {-1, -1};

  // Elijo aleatoriamente uno de los descendientes
  int elegido = rand() % sucesores.size();

  // Saco el movimiento realizado comparando el tablero original con el elegido.
  std::pair<int, int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

  return Mov;
}

/**
 * @brief Algoritmo de resolución completa para estados de final de juego.
 * Determina si una posición está matemáticamente ganada, perdida o empatada.
 * @param tablero Estado a evaluar.
 * @param Mov [Salida] La jugada óptima encontrada.
 * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
 */
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int, int> &Mov)
{
  /* ============== Este trozo de código se tiene que quedar aquí  =============== */
  nodosVisitados++;
  /* ============== Empieza a partir de aquí tu implementación  =============== */

  int jugadorActual = tablero.getJugadorTurno();

  int ganador = tablero.comprobarGanador();

  if (ganador == jugadorActual)
  {
    return Resultado::VICTORIA;
  }
  else if (ganador != 0 and ganador != -1)
  {
    return Resultado::DERROTA;
  }
  else if (ganador == -1)
  {
    return Resultado::EMPATE;
  }

  std::vector<std::pair<Tablero, std::pair<int, int>>> sucesores = tablero.getSucesoresConMovimientos();

  if (sucesores.empty())
  {
    int ganadorDesempate = tablero.getGanadorDesempate();
    if (ganadorDesempate == jugadorActual)
      return Resultado::VICTORIA;
    if (ganadorDesempate == -1)
      return Resultado::EMPATE;
    return Resultado::DERROTA;
  }

  // para buscar la jugada menos mala
  bool posibleEmpate = false;
  std::pair<int, int> mejorMovEmpate = sucesores[0].second;

  Mov = sucesores[0].second;

  for (const auto &sucesor : sucesores)
  {
    const Tablero &hijo = sucesor.first;
    std::pair<int, int> movHijo = sucesor.second;

    std::pair<int, int> movDummy;
    Resultado resHijo = Status(hijo, movDummy);

    int jugadorHijo = hijo.getJugadorTurno();

    if (jugadorActual == jugadorHijo)
    {
      if (resHijo == Resultado::VICTORIA)
      {
        Mov = movHijo;
        return Resultado::VICTORIA;
      }
      else if (resHijo == Resultado::EMPATE)
      {
        posibleEmpate = true;
        mejorMovEmpate = movHijo;
      }
    }
    else
    {
      if (resHijo == Resultado::DERROTA)
      {
        Mov = movHijo;
        return Resultado::VICTORIA;
      }
      else if (resHijo == Resultado::EMPATE)
      {
        posibleEmpate = true;
        mejorMovEmpate = movHijo;
      }
    }
  }

  if (posibleEmpate)
  {
    Mov = mejorMovEmpate;
    return Resultado::EMPATE;
  }

  return Resultado::DERROTA;
}

/**
 * @brief Implementación del algoritmo Minimax clásico.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado.
 */
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int, int> &Mov)
{
  /* ============== Este trozo de código se tiene que quedar aquí  =============== */
  nodosVisitados++;
  if (abortarBanda)
    return 0;

  if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos)
  {
    abortarBanda = true;
    return 0;
  }
  /* ============== Empieza a partir de aquí tu implementación  =============== */
  int jugadorActual = tablero.getJugadorTurno();
  bool soyMax = (jugadorActual == this->id);

  // compruebo si ha acabado el juego antes de llegar al final
  int ganador = tablero.comprobarGanador();

  if (ganador == this->id)
    return GANAR;
  if (ganador != 0 and ganador != -1)
    return PERDER;
  if (ganador == -1)
    return 0.0;

  // compruebo si se ha alcanzado el limte de profundidad
  if (profundidad >= prof_Max)
  {
    return heuristica(tablero);
  }

  auto sucesores = tablero.getSucesoresConMovimientos();

  // tablero lleno, no se generan hijos => evaluamos por puntos el desempate
  if (sucesores.empty())
  {
    int ganadorDesempate = tablero.getGanadorDesempate();
    if (ganadorDesempate == this->id)
      return GANAR;
    if (ganadorDesempate != 0 and ganadorDesempate != -1)
      return PERDER;
    return 0.0;
  }

  double mejorValor = soyMax ? MenosInfinito : MasInfinito;
  Mov = sucesores[0].second;

  for (const auto &sucesor : sucesores)
  {
    if (abortarBanda)
      return 0;

    const Tablero &hijo = sucesor.first;
    std::pair<int, int> movHijo = sucesor.second;

    std::pair<int, int> movDummy;
    double valorHijo = minimax(hijo, profundidad + 1, prof_Max, movDummy);

    if (soyMax)
    {
      if (valorHijo > mejorValor)
      {
        mejorValor = valorHijo;
        Mov = movHijo;
      }
    }
    else
    {
      if (valorHijo < mejorValor)
      {
        mejorValor = valorHijo;
        Mov = movHijo;
      }
    }
  }

  return mejorValor;
}

/**
 * @brief Punto de entrada para el juego inteligente.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida por el algoritmo de búsqueda.
 */
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero &tablero)
{
  std::pair<int, int> Mov;

  double valor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
  std::cout << "Valor Minimax: " << valor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
  return Mov;
}

/**
 * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param alfa Valor mínimo garantizado para el jugador MAX.
 * @param beta Valor máximo garantizado para el jugador MIN.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado tras la poda.
 */
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int, int> &Mov)
{
  /* ============== Este trozo de código se tiene que quedar aquí  =============== */
  nodosVisitados++;
  if (abortarBanda)
    return 0;

  if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos)
  {
    abortarBanda = true;
    return 0;
  }
  /* ============== Empieza a partir de aquí tu implementación  =============== */

  // 1. Identificamos si en este nodo nos toca jugar a nosotros (MAX) o al rival (MIN)
  int jugadorActual = tablero.getJugadorTurno();
  bool soyMax = (jugadorActual == this->id);

  // 2. CASOS BASE: Comprobamos si el juego ha terminado
  Tablero copiaTablero = tablero; // Copia por seguridad (para evitar errores con const)
  int ganador = copiaTablero.comprobarGanador();

  if (ganador == this->id)
    return GANAR;
  if (ganador != 0 && ganador != -1)
    return PERDER;
  if (ganador == -1)
    return 0.0; // Empate

  // 3. CASO BASE: Comprobamos si hemos alcanzado el límite de profundidad
  if (profundidad >= prof_Max)
  {
    return heuristica(tablero);
  }

  // 4. GENERAMOS LOS SUCESORES (movimientos posibles)
  auto sucesores = tablero.getSucesoresConMovimientos();

  // Si el tablero está lleno y no se pueden hacer más movimientos
  if (sucesores.empty())
  {
    int ganadorDesempate = tablero.getGanadorDesempate();
    if (ganadorDesempate == this->id)
      return GANAR;
    if (ganadorDesempate != 0 && ganadorDesempate != -1)
      return PERDER;
    return 0.0;
  }

  // 5. INICIALIZAMOS VARIABLES
  double mejorValor = soyMax ? MenosInfinito : MasInfinito;
  Mov = sucesores[0].second; // Movimiento por defecto por si acaso

  // 6. BUCLE ALFA-BETA
  for (const auto &sucesor : sucesores)
  {
    // Chequeo de seguridad por si nos quedamos sin tiempo dentro del bucle
    if (abortarBanda)
      return 0;

    const Tablero &hijo = sucesor.first;
    std::pair<int, int> movHijo = sucesor.second;

    std::pair<int, int> movDummy; // Variable temporal para la recursividad
    double valorHijo = alfaBeta(hijo, profundidad + 1, prof_Max, alfa, beta, movDummy);

    if (soyMax)
    {
      // MAX intenta maximizar el valor
      if (valorHijo > mejorValor)
      {
        mejorValor = valorHijo;
        Mov = movHijo; // Guardamos el movimiento que nos da este mejor valor
      }
      // Actualizamos alfa (lo mínimo garantizado para MAX)
      if (mejorValor > alfa)
      {
        alfa = mejorValor;
      }
      // PODA: Si lo que tengo garantizado (alfa) es mejor que el límite del rival (beta), corto
      if (alfa >= beta)
      {
        break;
      }
    }
    else
    {
      // MIN intenta minimizar el valor
      if (valorHijo < mejorValor)
      {
        mejorValor = valorHijo;
        Mov = movHijo; // Guardamos el movimiento que más nos perjudica (para preverlo)
      }
      // Actualizamos beta (lo máximo a lo que MIN está dispuesto a ceder)
      if (mejorValor < beta)
      {
        beta = mejorValor;
      }
      // PODA: Si el límite de MIN (beta) es peor que lo que MAX ya tiene garantizado (alfa), corto
      if (alfa >= beta)
      {
        break;
      }
    }
  }

  return mejorValor;

  return 0;
}



/**
 * @brief Función heurística para evaluar la calidad de un tablero.
 * @param tablero Estado a evaluar.
 * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
 */
double AgenteEstudiante::heuristica(const Tablero &tablero)
{
  switch (numHeuristica)
  {
  case 0:
    return heuristicaPrueba(tablero);
    break;
  case 1:
    return heuristica1(tablero);
    break;
  case 2:
    return heuristica2(tablero);
    break;
  default:
    return heuristica1(tablero);
  }
}


double AgenteEstudiante::heuristicaPrueba(const Tablero &tablero)
{
  // n es el número de fichas en línea para ganar.
  int n = tablero.getNParaGanar();
  int oponente = (id == 1) ? 2 : 1;
  double score_positivo = 0;

  double score_negativo = 0;

  for (int f = 0; f < tablero.getFilas(); f++)
  {
    for (int c = 0; c < tablero.getColumnas(); c++)
    {
      if (tablero.getCelda(f, c) != 0)
      {
        int valor = tablero.getFilas() - abs(f - (tablero.getFilas() / 2)) + tablero.getColumnas() - abs(c - (tablero.getColumnas() / 2));
        if (tablero.getCelda(f, c) == id)
        {
          score_positivo += valor;
        }
        else
        {
          score_negativo += valor;
        }
      }
    }
  }

  return score_positivo - score_negativo;
}

double AgenteEstudiante::heuristica1(const Tablero &tablero)
{
  int oponente = (id == 1) ? 2 : 1;
  int n = tablero.getNParaGanar();
  int filas = tablero.getFilas();
  int columnas = tablero.getColumnas();
  double score = 0.0;

  // =====================================================================
  // 1. CASOS BASE ABSOLUTOS
  // =====================================================================
  int ganador = tablero.comprobarGanador();
  if (ganador == id)
    return GANAR;
  if (ganador != 0 && ganador != -1)
    return PERDER;
  if (ganador == -1)
    return 0.0; // Empate

  // =====================================================================
  // 2. PRE-CÁLCULO DE HUECOS LEGALES (La clave de la velocidad)
  // =====================================================================
  int faseActual = tablero.getFaseActual() % 3;
  int faseMia = (tablero.getJugadorTurno() == id) ? faseActual : (faseActual + 1) % 3;
  int faseRival = (tablero.getJugadorTurno() == oponente) ? faseActual : (faseActual + 1) % 3;

  // Usamos vectores dinámicos por si el tablero no es de 9x9
  std::vector<std::vector<bool>> jugableMia(filas, std::vector<bool>(columnas, false));
  std::vector<std::vector<bool>> jugableRival(filas, std::vector<bool>(columnas, false));

  // =====================================================================
  // 3. BUCLE ÚNICO: Centralidad, Casillas Especiales y Adyacencia
  // =====================================================================
  for (int f = 0; f < filas; f++)
  {
    for (int c = 0; c < columnas; c++)
    {
      int celda = tablero.getCelda(f, c);

      // A. PRE-CÁLCULO DE ADYACENCIA
      if (celda == 0)
      {
        bool tieneAdyacente = false;
        for (int df = -1; df <= 1 && !tieneAdyacente; df++)
        {
          for (int dc = -1; dc <= 1 && !tieneAdyacente; dc++)
          {
            if (df == 0 && dc == 0)
              continue;
            int nf = f + df, nc = c + dc;
            if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas)
            {
              if (tablero.getCelda(nf, nc) != 0)
                tieneAdyacente = true;
            }
          }
        }
        if (tieneAdyacente)
        {
          if ((f + c) % 3 == faseMia)
            jugableMia[f][c] = true;
          if ((f + c) % 3 == faseRival)
            jugableRival[f][c] = true;
        }
      }

      // B. CENTRALIDAD (Para fichas colocadas)
      if (celda != 0)
      {
        int distCentro = abs(f - filas / 2) + abs(c - columnas / 2);
        double valorCentro = 10.0 - distCentro;

        if (celda == id)
          score += valorCentro;
        else
          score -= (valorCentro * 1.5);
      }

      // C. CASILLAS ESPECIALES
      auto tipo = tablero.getTipoCelda(f, c);
      if (tipo == Tablero::TipoCelda::ROJO)
      {
        if (celda == id)
          score -= 50;
        else if (celda == oponente)
          score += 30;
      }
      else if (tipo == Tablero::TipoCelda::VERDE)
      {
        if (celda == id)
          score += 80;
        else if (celda == oponente)
          score -= 100;
      }
      else if (tipo == Tablero::TipoCelda::AMARILLO && celda == 0)
      {
        int misFichasCruz = 0, susFichasCruz = 0;
        for (int i = 0; i < filas; i++)
        {
          if (tablero.getCelda(i, c) == id)
            misFichasCruz++;
          else if (tablero.getCelda(i, c) == oponente)
            susFichasCruz++;
        }
        for (int j = 0; j < columnas; j++)
        {
          if (tablero.getCelda(f, j) == id)
            misFichasCruz++;
          else if (tablero.getCelda(f, j) == oponente)
            susFichasCruz++;
        }
        score += (susFichasCruz - misFichasCruz) * 15.0;
      }
    }
  }

  // =====================================================================
  // 4. LÍNEAS BÁSICAS (Fuerza bruta global)
  // =====================================================================
  int mis_5_raya = tablero.contarCombinaciones(n, id);
  int mis_4_raya = tablero.contarCombinaciones(n - 1, id);
  int mis_3_raya = tablero.contarCombinaciones(n - 2, id);

  int sus_5_raya = tablero.contarCombinaciones(n, oponente);
  int sus_4_raya = tablero.contarCombinaciones(n - 1, oponente);
  int sus_3_raya = tablero.contarCombinaciones(n - 2, oponente);

  score += mis_5_raya * 100000.0;
  score += mis_4_raya * 1000.0;
  score += mis_3_raya * 100.0;

  score -= sus_5_raya * 200000.0; 
  score -= sus_4_raya * 2500.0;   
  score -= sus_3_raya * 200.0;

  // =====================================================================
  // 5. VENTANAS DESLIZANTES TÁCTICAS (Sustituye por completo al profesor)
  // =====================================================================
  // Direcciones: Horizontal, Vertical, Diagonal Descendente, Diagonal Ascendente
  int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
  int amenazasDoblesRival = 0;

  for (int d = 0; d < 4; d++)
  {
    int df = dirs[d][0];
    int dc = dirs[d][1];

    for (int f = 0; f < filas; f++)
    {
      for (int c = 0; c < columnas; c++)
      {
        // Calculamos dónde terminaría la ventana de tamaño n
        int fFinal = f + df * (n - 1);
        int cFinal = c + dc * (n - 1);

        // Si la ventana se sale de los límites del tablero, pasamos a la siguiente casilla
        if (fFinal < 0 || fFinal >= filas || cFinal < 0 || cFinal >= columnas)
          continue;

        int mias = 0;
        int rivales = 0;
        int huecosJugablesMios = 0;
        int huecosJugablesRival = 0;

        // Recorremos las n casillas consecutivas de la ventana actual
        for (int k = 0; k < n; k++)
        {
          int ff = f + df * k;
          int cc = c + dc * k;
          int v = tablero.getCelda(ff, cc);

          if (v == id)
          {
            mias++;
          }
          else if (v == oponente)
          {
            rivales++;
          }
          else
          {
            // Si la casilla está vacía, usamos nuestras matrices booleanas ultra-rápidas
            if (jugableMia[ff][cc])
              huecosJugablesMios++;
            if (jugableRival[ff][cc])
              huecosJugablesRival++;
          }
        }

        // --- A. EVALUAR AMENAZAS PROPIAS (Ventana limpia sin fichas rivales) ---
        if (rivales == 0 && mias > 0)
        {
          // Ventana con 4 piezas nuestras y al menos 1 hueco legal en nuestra fase
          if (mias == n - 1 && huecosJugablesMios >= 1)
          {
            score += 5000.0;
          }
          // Ventana con 3 piezas nuestras y al menos 2 huecos legales en nuestra fase (Ataque abierto)
          else if (mias == n - 2 && huecosJugablesMios >= 2)
          {
            score += 1500.0;
          }
        }

        // --- B. EVALUAR AMENAZAS DEL RIVAL (Ventana limpia sin fichas nuestras) ---
        if (mias == 0 && rivales > 0)
        {
          // Peligro crítico: El rival tiene 4 en raya y puede colocar la quinta legalmente
          if (rivales == n - 1 && huecosJugablesRival >= 1)
          {
            score -= 50000.0; // Bloqueo obligatorio
          }
          // Peligro táctico: El rival tiene 3 en raya con 2 extremos/huecos legales futuros libres
          else if (rivales == n - 2 && huecosJugablesRival >= 2)
          {
            score -= 30000.0; // ¡Caza al vuelo el tenedor del Ninja 3!
            amenazasDoblesRival++;
          }
          // Peligro menor: Línea de 3 con solo 1 hueco legal disponible
          else if (rivales == n - 2 && huecosJugablesRival == 1)
          {
            score -= 800.0;
          }
        }
      }
    }
  }

  // Si en el escaneo detectamos que el rival nos ha tendido un lazo con múltiples amenazas dobles
  if (amenazasDoblesRival >= 2)
    score -= 80000.0;

  return score;
}

double AgenteEstudiante::heuristica2(const Tablero &tablero)
{
  int oponente = (id == 1) ? 2 : 1;
  double puntuacion_final = 0.0;

  // =======================================================
  // CAPA 1: AMENAZAS CRÍTICAS (Conexión de Líneas)
  // =======================================================
  double mis_4 = tablero.contarCombinaciones(4, id);
  double mis_3 = tablero.contarCombinaciones(3, id);
  double mis_2 = tablero.contarCombinaciones(2, id);

  double sus_4 = tablero.contarCombinaciones(4, oponente);
  double sus_3 = tablero.contarCombinaciones(3, oponente);
  double sus_2 = tablero.contarCombinaciones(2, oponente);

  double score_lineas_miaso = (mis_4 * 10000) + (mis_3 * 100) + (mis_2 * 10);
  double score_lineas_suyas = (sus_4 * 15000) + (sus_3 * 150) + (sus_2 * 15);
  puntuacion_final += (score_lineas_miaso - score_lineas_suyas);

  // =======================================================
  // CAPA 2: CONTROL POSICIONAL Y CASILLAS ESPECIALES
  // =======================================================
  double score_posicional = 0.0;
  double score_especial = 0.0;

  for (int f = 0; f < tablero.getFilas(); f++)
  {
    for (int c = 0; c < tablero.getColumnas(); c++)
    {

      // 1. Puntuación por control del centro (tu código original)
      if (tablero.getCelda(f, c) != 0)
      {
        double valor_centro = tablero.getFilas() - abs(f - (tablero.getFilas() / 2)) +
                              tablero.getColumnas() - abs(c - (tablero.getColumnas() / 2));
        if (tablero.getCelda(f, c) == id)
        {
          score_posicional += valor_centro;
        }
        else
        {
          score_posicional -= valor_centro;
        }
      }
      // 2. Análisis de casillas especiales VACÍAS (Aún activables) [cite: 392-393]
      else
      {
        auto tipo = tablero.getTipoCelda(f, c);

        // --- EVALUACIÓN DE BOMBAS ---
        if (tipo == Tablero::TipoCelda::AMARILLO)
        {
          int mis_fichas_peligro = 0;
          int sus_fichas_peligro = 0;

          // Contamos cuántas fichas hay en la "cruz" de detonación de esta bomba
          for (int i = 0; i < tablero.getFilas(); i++)
          {
            if (tablero.getCelda(i, c) == id)
              mis_fichas_peligro++;
            else if (tablero.getCelda(i, c) == oponente)
              sus_fichas_peligro++;
          }
          for (int j = 0; j < tablero.getColumnas(); j++)
          {
            if (tablero.getCelda(f, j) == id)
              mis_fichas_peligro++;
            else if (tablero.getCelda(f, j) == oponente)
              sus_fichas_peligro++;
          }

          // Si tengo muchas fichas alineadas aquí, el Ninja la detonará. ¡Resta puntos!
          score_especial -= (mis_fichas_peligro * 200);
          // Si el Ninja tiene sus fichas aquí, nosotros podemos detonarla. ¡Suma puntos!
          score_especial += (sus_fichas_peligro * 200);
        }
        // --- EVALUACIÓN DE MÍSTICAS Y SABOTAJES ---
        else if (tipo == Tablero::TipoCelda::VERDE)
        {
          score_especial += 15; // Las zonas con turnos extra libres son prometedoras
        }
        else if (tipo == Tablero::TipoCelda::ROJO)
        {
          score_especial -= 10; // Las zonas minadas son peligrosas
        }
      }
    }
  }

  // Añadimos estos scores secundarios con menos peso para que no eclipsen a las líneas
  puntuacion_final += (score_posicional * 0.5) + score_especial;

  // =======================================================
  // CAPA 3: CONTEXTO DE TURNO Y "JAQUE MATE" INEVITABLE
  // =======================================================
  int turno_actual = tablero.getJugadorTurno();

  // Si el oponente tiene un 4 en raya y es SU turno, ya hemos perdido.
  if (sus_4 > 0 && turno_actual == oponente)
    return -999999.0;

  // DOBLE AMENAZA: Si el oponente tiene DOS líneas de 4, estamos muertos
  // aunque sea nuestro turno, porque solo podremos bloquearle una de las dos.
  if (sus_4 > 1 && turno_actual == id)
    return -999999.0;

  // Si yo tengo un 4 en raya y es mi turno, ¡victoria asegurada!
  if (mis_4 > 0 && turno_actual == id)
    return 999999.0;

  return puntuacion_final;
}
