function doGet(e) { 
  if (e.parameter == 'undefined') {
    return ContentService.createTextOutput( "No parameters");
  }

  var sheet_id = '1hxabuczRZIIJzgDkfBVMJcOoLHTuhKuI-8rgog015v0'; // Spreadsheet ID
  var sheet = SpreadsheetApp.openById(sheet_id).getSheetByName( "Data");
  var newRow = sheet.getLastRow() + 1; 
  var rowData = [];
  
  var Curr_Date = new Date();
  rowData[0] = newRow; 

  rowData[1] = Curr_Date; 

  var Curr_Time = Utilities.formatDate(Curr_Date, "Europe/Athens", 'HH:mm:ss');
  rowData[2] = Curr_Time;

  result = '';
  for (var param in e.parameter) {
      var value = stripQuotes(e.parameter[param]);
      value = value.replace(".", ",");

      switch (param) {
      case 'tem1':
        rowData[3] = value; // Temperature in column C
        break;
      case 'tem2':
        rowData[4] = value; // Temperature in column D
        break;
      case 'tem3':
        rowData[5] = value; // Temperature in column E
        break;
      case 'hum1':
        rowData[6] = value; // Humidity in column F
        break;
      case 'hum2':
        rowData[7] = value; // Humidity in column G
        break;
      case 'hum3':
        rowData[8] = value; // Humidity in column H
        break;
      case 'wat':
        rowData[9] = value; // Water level in column I
        break;        
      case 'wei':
        rowData[10] = value; // Weight in column J
        break;
      case 'mot':
        rowData[11] = value; // Moter time in column K
        break;
      case 'lig1':
        rowData[12] = value; // Light in column L
        break;
      case 'lig2':
        rowData[13] = value; // Light in column M
        break;
      case 'lig3':
        rowData[14] = value; // Light in column N
        break;
      default:
        result += "unsupported parameter: " + param;
        break;
      }
  }
  if( result == '') {
    result = 'ok';
  }

  var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
  newRange.setValues([rowData]);

  SaveLastData( sheet_id, rowData);

  sheet = SpreadsheetApp.openById(sheet_id).getSheetByName( "Params");
  result = "ok|" + sheet.getRange('b1').getValue( );
  result += "|" + sheet.getRange('b2').getValue( );
  result += "|" + sheet.getRange('b3').getValue( ) + "\n";

  return ContentService.createTextOutput( result);
}

function stripQuotes( value ) {
  return value.replace(/^["']|['"]$/g, "");
}

function SaveLastData( sheet_id, inpData) {
  var sheet = SpreadsheetApp.openById(sheet_id).getSheetByName( "Last");

  for( j = 1; j < 15; j++) {
    var newRange = sheet.getRange(j, 2, 1, 1);
    var rowData = [];
    rowData[ 0] = inpData[ j];
    newRange.setValues([rowData]);
  }
}
