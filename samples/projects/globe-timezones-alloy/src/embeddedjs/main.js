import Poco from "commodetto/Poco";

const render = new Poco(screen);
const fontSmall = new render.Font("Gothic-Bold", 18);
const fontTiny = new render.Font("Gothic-Bold", 14);

const C = {
  bg: render.makeColor(4, 10, 24),
  ocean: render.makeColor(20, 90, 170),
  grid: render.makeColor(100, 170, 230),
  city: render.makeColor(255, 220, 110),
  text: render.makeColor(235, 245, 255)
};

const CITIES = [
  { name: "NYC", lat: 40.7128, lon: -74.0060, offset: -5 },
  { name: "LON", lat: 51.5074, lon: -0.1278, offset: 0 },
  { name: "DXB", lat: 25.2048, lon: 55.2708, offset: 4 },
  { name: "DEL", lat: 28.6139, lon: 77.2090, offset: 5.5 },
  { name: "TYO", lat: 35.6762, lon: 139.6503, offset: 9 },
  { name: "SYD", lat: -33.8688, lon: 151.2093, offset: 10 },
  { name: "LAX", lat: 34.0549, lon: -118.2426, offset: -8 }
];

const DEG = Math.PI / 180;

function cityTime(now, offset) {
  const utcMillis = now.getTime() + now.getTimezoneOffset() * 60000;
  const cityDate = new Date(utcMillis + offset * 3600000);
  return `${String(cityDate.getHours()).padStart(2, "0")}:${String(cityDate.getMinutes()).padStart(2, "0")}`;
}

function plotPoint(color, x, y, size = 1) {
  render.fillRectangle(color, x | 0, y | 0, size, size);
}

function drawCircleDots(cx, cy, r, color) {
  for (let a = 0; a < 360; a += 3) {
    const rad = a * DEG;
    plotPoint(color, cx + Math.cos(rad) * r, cy + Math.sin(rad) * r);
  }
}

function project(latDeg, lonDeg, rot) {
  const lat = latDeg * DEG;
  const lon = lonDeg * DEG + rot;
  const x = Math.cos(lat) * Math.sin(lon);
  const y = Math.sin(lat);
  const z = Math.cos(lat) * Math.cos(lon);
  return { x, y, z };
}

function draw(event) {
  const now = event.date;
  const w = render.width;
  const h = render.height;
  const cx = w >> 1;
  const cy = (h >> 1) - 8;
  const r = Math.min(w, h) * 0.32;
  const rot = (now.getSeconds() / 60) * Math.PI * 2;

  render.begin();
  render.fillRectangle(C.bg, 0, 0, w, h);

  drawCircleDots(cx, cy, r, C.ocean);

  for (let lat = -60; lat <= 60; lat += 30) {
    for (let lon = -180; lon < 180; lon += 6) {
      const p = project(lat, lon, rot);
      if (p.z > 0) plotPoint(C.grid, cx + p.x * r, cy - p.y * r * 0.9);
    }
  }

  for (let lon = -150; lon <= 150; lon += 30) {
    for (let lat = -85; lat <= 85; lat += 5) {
      const p = project(lat, lon, rot);
      if (p.z > 0) plotPoint(C.grid, cx + p.x * r, cy - p.y * r * 0.9);
    }
  }

  const local = `${String(now.getHours()).padStart(2, "0")}:${String(now.getMinutes()).padStart(2, "0")}`;
  const localW = render.getTextWidth(local, fontSmall);
  render.drawText(local, fontSmall, C.text, (w - localW) / 2, 6);

  let rowY = h - 44;
  const cols = [6, w / 2 + 2];
  CITIES.forEach((city, i) => {
    const p = project(city.lat, city.lon, rot);
    if (p.z > 0) {
      const px = cx + p.x * r;
      const py = cy - p.y * r * 0.9;
      plotPoint(C.city, px - 1, py - 1, 3);
      render.drawText(city.name, fontTiny, C.city, px + 4, py - 6);
    }

    const col = i % 2;
    if (i > 0 && col === 0) rowY += 12;
    const line = `${city.name} ${cityTime(now, city.offset)}`;
    render.drawText(line, fontTiny, C.text, cols[col], rowY);
  });

  render.end();
}

watch.addEventListener("secondchange", draw);
