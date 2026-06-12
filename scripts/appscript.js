function doPost(e) {

  // Llamado a la hoja donde se almacenan los datos. Debe verificarse el nombre de la hoja.
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName('prueba'); 

  // Añadir marcas temporales (etiqueta, fecha y hora)
  var datetime = new Date().toLocaleString('es-AR');
  var date = new Date().toLocaleDateString('es-AR');
  var time = new Date().toLocaleTimeString('es-AR');

  // Codificar parámetros enviados desde el ESP32
  var params = e.parameter;
  var temp = params.temp;
  
  // Convertir el valor de temperatura a número
  var tempDec = parseFloat(temp); 

  // Omitir valores nulos por desconexion del termómetro o fallas en la comunicación
  if (tempDec >= -50) {
    
    // Dar formato con coma como el separador decimal
    var tempDec = Utilities.formatString("%.2f", tempDec).replace(".", ",");
    
    // Agregar nueva fila
    sheet.appendRow([
      datetime,
      date,
      time,
      tempDec,
      ]);
    }
  
  return ContentService.createTextOutput("OK");
}
