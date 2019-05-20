
let cacheName = "v1";
var CACHE_NAME = 'static-cache';
var urlsToCache = [
  '.',
  'index.html',
  "./node_modules/bootstrap/dist/css/bootstrap.min.css",
  "./node_modules/jquery/dist/jquery.slim.min.js",
  "./node_modules/popper.js/dist/umd/popper.min.js",
  "./node_modules/bootstrap/dist/js/bootstrap.min.js",
  "./node_modules/moment/moment.js",
  "./node_modules/chart.js/dist/Chart.js",
  "./node_modules/chartjs-plugin-zoom/chartjs-plugin-zoom.min.js",
  "./node_modules/chartjs-plugin-streaming/dist/chartjs-plugin-streaming.min.js"
];

self.addEventListener('install', function(event) {
  event.waitUntil(
    caches.open(CACHE_NAME)
    .then(function(cache) {
      return cache.addAll(urlsToCache);
    })
  );
});
self.addEventListener('fetch', function(event) {
  event.respondWith(
    caches.match(event.request)
    .then(function(response) {
      return response || fetchAndCache(event.request);
    })
  );
});

function fetchAndCache(url) {
  return fetch(url)
  .then(function(response) {
    // Check if we received a valid response
    if (!response.ok) {
      throw Error(response.statusText);
    }
    return caches.open(CACHE_NAME)
    .then(function(cache) {
      cache.put(url, response.clone());
      return response;
    });
  })
  .catch(function(error) {
    console.log('Request failed:', error);
    // You could return a custom offline 404 page here
  });
}
