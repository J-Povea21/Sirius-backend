export default async function routes(fastify, options) {

    fastify.get("/", async (req, res) => {
        return res.sendFile('test.html');
    });

    fastify.get("/test", async (req, res) => {
        return res.sendFile('test-2.html');
    });

}