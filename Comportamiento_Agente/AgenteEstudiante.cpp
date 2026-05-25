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

  for (const auto &sucesor : sucesores) //auto es, Vector de {Tablero,Movimiento}
  {
    if (abortarBanda)
      return 0;

    const Tablero &hijo = sucesor.first;
    std::pair<int, int> movHijo = sucesor.second;

    std::pair<int, int> movTmp;
    double valorHijo = minimax(hijo, profundidad + 1, prof_Max, movTmp);

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

  // saber si es min o max
  int jugadorActual = tablero.getJugadorTurno();
  bool soyMax = (jugadorActual == this->id);

  //hemos ganado
  int ganador = tablero.comprobarGanador();

  if (ganador == this->id)
    return GANAR;
  if (ganador != 0 && ganador != -1)
    return PERDER;
  if (ganador == -1)
    return 0.0; // Empate

  //profundidad
  if (profundidad >= prof_Max)
  {
    return heuristica(tablero);
  }

  // GENERAMOS SUCESORES
  auto sucesores = tablero.getSucesoresConMovimientos();

  // si el tablero está lleno y no se pueden hacer más movimientos, miramos el desempate
  if (sucesores.empty())
  {
    int ganadorDesempate = tablero.getGanadorDesempate();
    if (ganadorDesempate == this->id)
      return GANAR;
    if (ganadorDesempate != 0 && ganadorDesempate != -1)
      return PERDER;
    return 0.0;
  }

  
  double mejorValor = soyMax ? MenosInfinito : MasInfinito;
  Mov = sucesores[0].second;

  //BUCLE ALFA-BETA
  for (const auto &sucesor : sucesores) // auto es de tipo de dato std::vector<std::pair<Tablero, std::pair<int,int>>> VECTOR DE {TABLERO,MOVIMIENTO}
  {
    //mirar tiempo
    if (abortarBanda)
      return 0;

    const Tablero &hijo = sucesor.first;
    std::pair<int, int> movHijo = sucesor.second;

    std::pair<int, int> movTmp; //variable temporal, la utilizamos en la recursividad
    double valorHijo = alfaBeta(hijo, profundidad + 1, prof_Max, alfa, beta, movTmp);

    if (soyMax)
    {
      // MAX intenta maximizar el valor
      if (valorHijo > mejorValor)
      {
        mejorValor = valorHijo;
        Mov = movHijo;
      }
      // cctualizamos alfa (lo mínimo garantizado para MAX)
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
        Mov = movHijo;
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

  // casos base absolutos
  int ganador = tablero.comprobarGanador();
  if (ganador == id)
    return GANAR;
  if (ganador != 0 && ganador != -1)
    return PERDER;
  if (ganador == -1)
    return 0.0; // Empate

  // Preferimos el centro, miramos las casillas especiales y vemos si hay adyacencia
  for (int f = 0; f < filas; f++)
  {
    for (int c = 0; c < columnas; c++)
    {
      int celda = tablero.getCelda(f, c);

      //Para fichas colocadas
      if (celda != 0)
      {
        int distCentro = abs(f - filas / 2) + abs(c - columnas / 2);
        double valorCentro = 10.0 - distCentro;

        if (celda == id)
          score += valorCentro;
        else
          score -= (valorCentro * 1.5);
      }

      // casillas especiales
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
  return score;
}

double AgenteEstudiante::heuristica2(const Tablero &tablero)
{
  //la heuristica 1 me da para los 4 ninjas, y el tiempo que tarda en tomar la decision no es muy elevado,
  //por lo tanto, borro las heuristicas con las que he probado y me quedo con la 1, que es definitiva
  return heuristica1(tablero);
}
