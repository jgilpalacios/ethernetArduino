/**
 * El acceso a /preguntas es de tipo estatico ya que la pagina es simple y no requiere proceso
 * El acceso a /respuesta es mediante app.get() ya que varía en función de la respuesta dado
 * El parámetro oculto que permite ver que formulario se ha contestado es formulario que traerá
 * valores America si se responde al primero y Portugal si se responde al segundo.
 */

var express = require('express');
var path = require('path');
var app = express();

// Middleware de acceso a páginas Web estáticas
//   -> root = directorio 'public'
//   -> __dirnme: nombre del directorio de ejecución

app.use(express.static(path.join(__dirname, 'PAGINA')));

app.get('/IFRAME',function (req, res){
  var mensaje='100#100#100#100#100#100##1#0#1#1#1#0#1';
  res.send(mensaje);
});

app.get('/CONFIGURA',function (req, res){
  var mensaje='1#1#0#0#0#1#1';
  res.send(mensaje);
});



app.listen(8000);

console.log('Esperando solicitudes en el puerto 8000');