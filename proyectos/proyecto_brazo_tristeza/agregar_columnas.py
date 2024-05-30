# se recibe fecha formato mm/dd/aaa y se pasa a 
# aaaammdd
def formatear_fecha(una_fecha):
	datos = una_fecha.split("/")
	anio = int(datos[2])
	mes = int(datos[0])
	dia = int(datos[1])
	otra_fecha = anio * 10000 + mes * 100 + dia
	return(otra_fecha)


if __name__ == '__main__':
	
	# abro archivo de entrada y de salida
    file_in = open("bosque_brazo_tristeza.asc", "r")
    file_out = open("bosque_brazo_tristeza_ok.asc", "w")

    # procesamiento de datos
    for x in range(1,7):
        line = file_in.readline()
        file_out.write(line)

    line = file_in.readline()
    while line:
        # agrego las columnas que faltan con valor 0
        line_out = " -1 -1" + line
        # escribo en el archivo de salida
        file_out.write(line_out)

	 #leo proxima linea
        line = file_in.readline()

   
    # fin procesamiento cierre de archivos
    file_in.close()
    file_out.close()
