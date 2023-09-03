export default async function routes(fastify, options) {

    fastify.get("/", async (req, res) => {
        return res.sendFile('test.html');
    });

}