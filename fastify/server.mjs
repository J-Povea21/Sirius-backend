import fastify from "fastify";
import fastifyIO from "fastify-socket.io";

const app = fastify();

app.register(fastifyIO);

