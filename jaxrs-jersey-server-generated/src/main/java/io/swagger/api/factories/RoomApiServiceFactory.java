package io.swagger.api.factories;

import io.swagger.api.RoomApiService;
import io.swagger.api.impl.RoomApiServiceImpl;

@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-18T18:02:56.242Z[GMT]")public class RoomApiServiceFactory {
    private final static RoomApiService service = new RoomApiServiceImpl();

    public static RoomApiService getRoomApi() {
        return service;
    }
}
