program teste_binomios;
var
  res : integer;
begin
  { CASO 1: Binomio Valido (Deve passar) }
  res := (var_um + 2)^5;

  { CASO 2: Binomio Invalido (Expoente negativo) }
  { O seu lexico deve rejeitar isso como binomio e retornar tokens soltos }
  { O sintatico vai reclamar do '^' solto ou do '-' }
  res := (x + y)^-1;
end.