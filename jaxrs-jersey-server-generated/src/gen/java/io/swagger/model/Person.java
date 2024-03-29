/*
 * Plathea
 * Rest APIs Plathea
 *
 * OpenAPI spec version: 1.0.1-oas3
 * 
 *
 * NOTE: This class is auto generated by the swagger code generator program.
 * https://github.com/swagger-api/swagger-codegen.git
 * Do not edit the class manually.
 */

package io.swagger.model;

import java.util.Objects;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;
import io.swagger.model.Entity;
import io.swagger.model.Position;
import io.swagger.v3.oas.annotations.media.Schema;
import javax.validation.constraints.*;

/**
 * Person
 */
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaJerseyServerCodegen", date = "2018-11-18T18:02:56.242Z[GMT]")public class Person extends Entity  {
  @JsonProperty("identified")
  private Boolean identified = null;

  public Person identified(Boolean identified) {
    this.identified = identified;
    return this;
  }

  /**
   * Get identified
   * @return identified
   **/
  @JsonProperty("identified")
  @Schema(description = "")
  public Boolean isIdentified() {
    return identified;
  }

  public void setIdentified(Boolean identified) {
    this.identified = identified;
  }


  @Override
  public boolean equals(java.lang.Object o) {
    if (this == o) {
      return true;
    }
    if (o == null || getClass() != o.getClass()) {
      return false;
    }
    Person person = (Person) o;
    return Objects.equals(this.identified, person.identified) &&
        super.equals(o);
  }

  @Override
  public int hashCode() {
    return Objects.hash(identified, super.hashCode());
  }


  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append("class Person {\n");
    sb.append("    ").append(toIndentedString(super.toString())).append("\n");
    sb.append("    identified: ").append(toIndentedString(identified)).append("\n");
    sb.append("}");
    return sb.toString();
  }

  /**
   * Convert the given object to string with each line indented by 4 spaces
   * (except the first line).
   */
  private String toIndentedString(java.lang.Object o) {
    if (o == null) {
      return "null";
    }
    return o.toString().replace("\n", "\n    ");
  }
}
