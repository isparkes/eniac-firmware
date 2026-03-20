FROM node:22

WORKDIR /app
COPY package.json .
COPY package-lock.json .

RUN npm ci

COPY server.js .
COPY priceTracker.js .

CMD ["node", "server.js"]
