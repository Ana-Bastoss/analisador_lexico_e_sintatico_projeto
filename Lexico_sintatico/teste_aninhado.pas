program teste_profundidade;
var
  i, j, k : integer;
  flag : integer;
begin
  i := 0;
  flag := 1;
  
  while i < 10 do
  begin
    if flag = 1 then
    begin
      j := i * 2;
      if j > 10 then
        k := j - 5
      else
        k := j + 5
    end;
    
    i := i + 1
  end
end.