import Poco from "commodetto/Poco";
import parseBMF from "commodetto/parseBMF";
import parseRLE from "commodetto/parseRLE";
import Battery from "embedded:sensor/Battery";
import Location from "embedded:sensor/Location";

const render = new Poco(screen);

// Load a custom font from BMF resources
function getFont(name, size) {
    const font = parseBMF(new Resource(`${name}-${size}.fnt`));
    font.bitmap = parseRLE(new Resource(`${name}-${size}-alpha.bm4`));
    return font;
}

// Fonts
const timeFont = getFont("Jersey10-Regular", 56);
const dateFont = getFont("Jersey10-Regular", 24);
const smallFont = new render.Font("Gothic-Regular", 18);

// Colors
const black = render.makeColor(0, 0, 0);
const white = render.makeColor(255, 255, 255);
const green = render.makeColor(0, 170, 0);
const yellow = render.makeColor(255, 170, 0);
const red = render.makeColor(255, 0, 0);

// Day and month names
const DAYS = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
const MONTHS = ["Jan", "Feb", "Mar", "Apr", "May", "Jun",
                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];

// Precompute layout positions
const blockHeight = timeFont.height + dateFont.height;
const timeY = (render.height - blockHeight) / 2;
const dateY = timeY + timeFont.height;

// Store latest time for redraws triggered by other events
let lastDate = new Date();

// Weather data
let weather = null;
let latitude = null;
let longitude = null;

// Battery state
let batteryPercent = 100;

const battery = new Battery({
    onSample() {
        batteryPercent = this.sample().percent;
        drawScreen();
    }
});
batteryPercent = battery.sample().percent;

// Connection state
let isConnected = true;

function checkConnection() {
    isConnected = watch.connected.app;
    drawScreen();
}
watch.addEventListener("connected", checkConnection);
checkConnection();

// Map Open-Meteo weather codes to descriptions
function getWeatherDescription(code) {
    if (code === 0) return "Clear";
    if (code <= 3) return "Cloudy";
    if (code <= 48) return "Fog";
    if (code <= 55) return "Drizzle";
    if (code <= 57) return "Fz. Drizzle";
    if (code <= 65) return "Rain";
    if (code <= 67) return "Fz. Rain";
    if (code <= 75) return "Snow";
    if (code <= 77) return "Snow Grains";
    if (code <= 82) return "Showers";
    if (code <= 86) return "Snow Shwrs";
    if (code === 95) return "T-Storm";
    if (code <= 99) return "T-Storm";
    return "Unknown";
}

// Get location from the Location sensor
let location = null;

function requestLocation() {
    location = new Location({
        onSample() {
            const sample = this.sample();
            console.log("Got location: " + sample.latitude + ", " + sample.longitude);
            this.close();
            fetchWeather(sample.latitude, sample.longitude);
        }
    });
}

async function fetchWeather(latitude, longitude) {
    try {
        const url = new URL("http://api.open-meteo.com/v1/forecast");
        url.search = new URLSearchParams({
            latitude,
            longitude,
            current: "temperature_2m,weather_code"
        });

        console.log("Fetching weather...");
        const response = await fetch(url);
        const data = await response.json();

        weather = {
            temp: Math.round(data.current.temperature_2m),
            conditions: getWeatherDescription(data.current.weather_code)
        };

        console.log("Weather: " + weather.temp + "C, " + weather.conditions);
        drawScreen();

    } catch (e) {
        console.log("Weather fetch error: " + e);
    }
}

function drawBatteryBar() {
    const barWidth = (render.width / 2) | 0;
    const barX = ((render.width - barWidth) / 2) | 0;
    const barY = render.height < 180 ? 6 : 20;
    const barHeight = 8;

    // Draw border
    render.fillRectangle(white, barX, barY, barWidth, barHeight);
    render.fillRectangle(black, barX + 1, barY + 1, barWidth - 2, barHeight - 2);

    // Choose color based on battery level
    let barColor;
    if (batteryPercent <= 20) {
        barColor = red;
    } else if (batteryPercent <= 40) {
        barColor = yellow;
    } else {
        barColor = green;
    }

    // Draw filled portion
    const fillWidth = ((batteryPercent * (barWidth - 4)) / 100) | 0;
    render.fillRectangle(barColor, barX + 2, barY + 2, fillWidth, barHeight - 4);
}

function drawScreen(event) {
    const now = event?.date ?? lastDate;
    if (event?.date) lastDate = event.date;

    render.begin();
    render.fillRectangle(black, 0, 0, render.width, render.height);

    // Draw battery bar at top
    drawBatteryBar();

    // Draw Bluetooth disconnect indicator below battery bar
    if (!isConnected) {
        const btStr = "X";
        const btWidth = render.getTextWidth(btStr, smallFont);
        const btY = render.height < 180 ? 16 : 30;
        render.drawText(btStr, smallFont, red,
            (render.width - btWidth) / 2, btY);
    }

    // Format time as HH:MM
    const hours = String(now.getHours()).padStart(2, "0");
    const minutes = String(now.getMinutes()).padStart(2, "0");
    const timeStr = `${hours}:${minutes}`;

    // Draw time centered
    let width = render.getTextWidth(timeStr, timeFont);
    render.drawText(timeStr, timeFont, white,
        (render.width - width) / 2, timeY);

    // Format date as "Mon Jan 01"
    const dayName = DAYS[now.getDay()];
    const monthName = MONTHS[now.getMonth()];
    const dateStr = `${dayName} ${monthName} ${String(now.getDate()).padStart(2, "0")}`;

    // Draw date centered below time
    width = render.getTextWidth(dateStr, dateFont);
    render.drawText(dateStr, dateFont, white,
        (render.width - width) / 2, dateY);

    // Draw weather at bottom
    const weatherY = render.height - smallFont.height - (render.height < 180 ? 6 : 20);
    if (weather) {
        const weatherStr = `${weather.temp}°C ${weather.conditions}`;
        width = render.getTextWidth(weatherStr, smallFont);
        render.drawText(weatherStr, smallFont, white,
            (render.width - width) / 2, weatherY);
    } else {
        const msg = "Loading...";
        width = render.getTextWidth(msg, smallFont);
        render.drawText(msg, smallFont, white,
            (render.width - width) / 2, weatherY);
    }

    render.end();
}

// Update every minute (fires immediately when registered)
watch.addEventListener("minutechange", drawScreen);

// Refresh weather every 30 minutes
watch.addEventListener("hourchange", requestLocation);
