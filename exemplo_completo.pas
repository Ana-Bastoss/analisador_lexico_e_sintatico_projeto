program exemplo_completo;
var
  contador, limite : integer;
  resultado : real;
begin
  { Inicializacao de variaveis }
  contador := 1;
  limite := 10;
  
  { Teste de Expressao Binomial (Sua feature especial!) }
  resultado := (x + 2)^3;

  { Teste de Loop }
  while contador < limite do
  begin
    { Teste de IF-ELSE e Operacoes }
    if contador > 5 then
      resultado := resultado + 2.5
    else
      resultado := resultado * 1.0;
      
    contador := contador + 1
  end
end.