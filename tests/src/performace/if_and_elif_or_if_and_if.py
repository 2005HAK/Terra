# Gera a sequência de elif e if e escreve no arquivo

# Abre o arquivo para escrita
with open('test_verification_ifs.py', 'w') as file:
    var = 10000

    file.write(f"var = {var}\n\n")

    # file.write("if var == 0:\n    pass\n")
    
    # for i in range(1, var + 1):
    #     file.write(f"elif var == {i}:\n    pass\n")
    
    for i in range(var + 1):
        file.write(f"if var == {i}:\n    pass\n")